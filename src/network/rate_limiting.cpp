#include <fc/network/rate_limiting.hpp>
#include <fc/network/tcp_socket_io_hooks.hpp>
#include <fc/network/tcp_socket.hpp>
#include <fc/network/ip.hpp>
#include <fc/fwd_impl.hpp>
#include <fc/asio.hpp>
#include <fc/log/logger.hpp>
#include <fc/io/stdio.hpp>
#include <fc/exception/exception.hpp>
#include <fc/thread/thread.hpp>

namespace fc 
{

  namespace detail
  {
    // data about a read or write we're managing
    class rate_limited_operation
    {
    public:
      size_t                        length;
      size_t                        permitted_length;
      promise<size_t>::ptr          completion_promise;

      rate_limited_operation(size_t length,
                             promise<size_t>::ptr&& completion_promise) :
        length(length),
        permitted_length(0),
        completion_promise(completion_promise)
      {}

      virtual void perform_operation() = 0;
    };

    class rate_limited_tcp_write_operation : public rate_limited_operation
    {
    public:
      boost::asio::ip::tcp::socket& socket;
      const char*                   buffer;

      rate_limited_tcp_write_operation(boost::asio::ip::tcp::socket& socket,
                                       const char* buffer,
                                       size_t length,
                                       promise<size_t>::ptr completion_promise) :
        rate_limited_operation(length, std::move(completion_promise)),
        socket(socket),
        buffer(buffer)
      {}
      virtual void perform_operation() override
      {
        asio::async_write_some(socket,
                               boost::asio::buffer(buffer, permitted_length),
                               completion_promise);
      }
    };
    class rate_limited_tcp_read_operation : public rate_limited_operation
    {
    public:
      boost::asio::ip::tcp::socket& socket;
      char*                         buffer;

      rate_limited_tcp_read_operation(boost::asio::ip::tcp::socket& socket,
                                      char* buffer,
                                      size_t length,
                                      promise<size_t>::ptr completion_promise) :
        rate_limited_operation(length, std::move(completion_promise)),
        socket(socket),
        buffer(buffer)
      {}
      virtual void perform_operation() override
      {
        asio::async_read_some(socket,
                              boost::asio::buffer(buffer, permitted_length),
                              completion_promise);
      }
    };

    struct is_operation_shorter
    {
      // less than operator designed to bring the shortest operations to the end
      bool operator()(const rate_limited_operation* lhs, const rate_limited_operation* rhs)
      { 
        return lhs->length > rhs->length; 
      }
    };

    class rate_limiting_group_impl : public tcp_socket_io_hooks
    {
    public:
      uint32_t _upload_bytes_per_second;
      uint32_t _download_bytes_per_second;

      microseconds _granularity; // how often to add tokens to the bucket
      uint32_t _read_tokens;
      uint32_t _unused_read_tokens; // gets filled with tokens for unused bytes (if I'm allowed to read 200 bytes and I try to read 200 bytes, but can only read 50, tokens for the other 150 get returned here)
      uint32_t _write_tokens;
      uint32_t _unused_write_tokens;

      typedef std::list<rate_limited_operation*> rate_limited_operation_list;
      rate_limited_operation_list _read_operations_in_progress;
      rate_limited_operation_list _read_operations_for_next_iteration;
      rate_limited_operation_list _write_operations_in_progress;
      rate_limited_operation_list _write_operations_for_next_iteration;

      time_point _last_read_iteration_time;
      time_point _last_write_iteration_time;

      future<void> _process_pending_reads_loop_complete;
      promise<void>::ptr _new_read_operation_available_promise;
      future<void> _process_pending_writes_loop_complete;
      promise<void>::ptr _new_write_operation_available_promise;

      rate_limiting_group_impl(uint32_t upload_bytes_per_second, uint32_t download_bytes_per_second);

      virtual size_t readsome(boost::asio::ip::tcp::socket& socket, char* buffer, size_t length) override;
      virtual size_t writesome(boost::asio::ip::tcp::socket& socket, const char* buffer, size_t length) override;

      void process_pending_reads();
      void process_pending_writes();
      void process_pending_operations(time_point& last_iteration_start_time,
                                      uint32_t& limit_bytes_per_second,
                                      rate_limited_operation_list& operations_in_progress,
                                      rate_limited_operation_list& operations_for_next_iteration,
                                      uint32_t& tokens,
                                      uint32_t& unused_tokens);
    };

    rate_limiting_group_impl::rate_limiting_group_impl(uint32_t upload_bytes_per_second, uint32_t download_bytes_per_second) :
      _upload_bytes_per_second(upload_bytes_per_second),
      _download_bytes_per_second(download_bytes_per_second),
      _granularity(milliseconds(50)),
      _read_tokens(_download_bytes_per_second),
      _unused_read_tokens(0),
      _write_tokens(_upload_bytes_per_second),
      _unused_write_tokens(0)
    {
    }

    size_t rate_limiting_group_impl::readsome(boost::asio::ip::tcp::socket&  socket, char* buffer, size_t length)
    {
      if (_download_bytes_per_second)
      {
        promise<size_t>::ptr completion_promise(new promise<size_t>());
        rate_limited_tcp_read_operation read_operation(socket, buffer, length, completion_promise);
        _read_operations_for_next_iteration.push_back(&read_operation);

        // launch the read processing loop it if isn't running, or signal it to resume if it's paused.
        if (!_process_pending_reads_loop_complete.valid() || _process_pending_reads_loop_complete.ready())
          _process_pending_reads_loop_complete = async([=](){ process_pending_reads(); });
        else if (_new_read_operation_available_promise)
          _new_read_operation_available_promise->set_value();

        size_t bytes_read = completion_promise->wait();
        _unused_read_tokens += read_operation.permitted_length - bytes_read;
        return bytes_read;
      }
      else
        return asio::read_some(socket, boost::asio::buffer(buffer, length));
    }
    size_t rate_limiting_group_impl::writesome(boost::asio::ip::tcp::socket&  socket, const char* buffer, size_t length)
    {
      if (_upload_bytes_per_second)
      {
        promise<size_t>::ptr completion_promise(new promise<size_t>());
        rate_limited_tcp_write_operation write_operation(socket, buffer, length, completion_promise);
        _write_operations_for_next_iteration.push_back(&write_operation);

        // launch the write processing loop it if isn't running, or signal it to resume if it's paused.
        if (!_process_pending_writes_loop_complete.valid() || _process_pending_writes_loop_complete.ready())
          _process_pending_writes_loop_complete = async([=](){ process_pending_writes(); });
        else if (_new_write_operation_available_promise)
          _new_write_operation_available_promise->set_value();

        size_t bytes_written = completion_promise->wait();
        _unused_write_tokens += write_operation.permitted_length - bytes_written;
        return bytes_written;
      }
      else
        return asio::write_some(socket, boost::asio::buffer(buffer, length));
    }
    void rate_limiting_group_impl::process_pending_reads()
    {
      for (;;)
      {
        process_pending_operations(_last_read_iteration_time, _download_bytes_per_second,
                                   _read_operations_in_progress, _read_operations_for_next_iteration, _read_tokens, _unused_read_tokens);

        _new_read_operation_available_promise = new promise<void>();
        try
        {
          if (_read_operations_in_progress.empty())
            _new_read_operation_available_promise->wait();
          else
            _new_read_operation_available_promise->wait(_granularity);
        }
        catch (const timeout_exception&)
        {
        }
        _new_read_operation_available_promise.reset();
      }
    }
    void rate_limiting_group_impl::process_pending_writes()
    {
      for (;;)
      {
        process_pending_operations(_last_write_iteration_time, _upload_bytes_per_second,
                                   _write_operations_in_progress, _write_operations_for_next_iteration, _write_tokens, _unused_write_tokens);

        _new_write_operation_available_promise = new promise<void>();
        try
        {
          if (_write_operations_in_progress.empty())
            _new_write_operation_available_promise->wait();
          else
            _new_write_operation_available_promise->wait(_granularity);
        }
        catch (const timeout_exception&)
        {
        }
        _new_write_operation_available_promise.reset();
      }
    }
    void rate_limiting_group_impl::process_pending_operations(time_point& last_iteration_start_time, 
                                                              uint32_t& limit_bytes_per_second,
                                                              rate_limited_operation_list& operations_in_progress,
                                                              rate_limited_operation_list& operations_for_next_iteration,
                                                              uint32_t& tokens,
                                                              uint32_t& unused_tokens)
    {
      // lock here for multithreaded
      std::copy(operations_for_next_iteration.begin(),
                operations_for_next_iteration.end(),
                std::back_inserter(operations_in_progress));
      operations_for_next_iteration.clear();

      // find out how much time since our last read/write
      time_point this_iteration_start_time = time_point::now();
      if (limit_bytes_per_second) // the we are limiting up/download speed
      {
        microseconds time_since_last_iteration = this_iteration_start_time - last_iteration_start_time;
        if (time_since_last_iteration > seconds(1))
          time_since_last_iteration = seconds(1);
        else if (time_since_last_iteration < microseconds(0))
          time_since_last_iteration = microseconds(0);
 
        tokens += (uint32_t)((limit_bytes_per_second * time_since_last_iteration.count()) / 1000000);
        tokens += unused_tokens;
        unused_tokens = 0;
        tokens = std::min(tokens, limit_bytes_per_second);

        if (tokens)
        {
          // sort the pending reads/writes in order of the number of bytes they need to write, smallest first
          std::vector<rate_limited_operation*> operations_sorted_by_length;
          operations_sorted_by_length.reserve(operations_in_progress.size());
          for (rate_limited_operation* operation_data : operations_in_progress)
            operations_sorted_by_length.push_back(operation_data);
          std::sort(operations_sorted_by_length.begin(), operations_sorted_by_length.end(), is_operation_shorter());

          // figure out how many bytes each reader/writer is allowed to read/write
          uint32_t bytes_remaining_to_allocate = tokens;
          while (!operations_sorted_by_length.empty())
          {
            uint32_t bytes_permitted_for_this_operation = bytes_remaining_to_allocate / operations_sorted_by_length.size();
            uint32_t bytes_allocated_for_this_operation = std::min(operations_sorted_by_length.back()->length, bytes_permitted_for_this_operation);
            operations_sorted_by_length.back()->permitted_length = bytes_allocated_for_this_operation;
            bytes_remaining_to_allocate -= bytes_allocated_for_this_operation;
            operations_sorted_by_length.pop_back();
          }
          tokens = bytes_remaining_to_allocate;

          // kick off the reads/writes in first-come order
          for (auto iter = operations_in_progress.begin(); iter != operations_in_progress.end();)
          {
            if ((*iter)->permitted_length > 0)
            {
              (*iter)->perform_operation();
              iter = operations_in_progress.erase(iter);
            }
            else
              ++iter;
          }
        }
      }
      else // down/upload speed is unlimited
      {
        // we shouldn't end up here often.  If the rate is unlimited, we should just execute
        // the operation immediately without being queued up.  This should only be hit if
        // we change from a limited rate to unlimited
        for (auto iter = operations_in_progress.begin(); 
             iter != operations_in_progress.end();
             ++iter)
        {
          (*iter)->permitted_length = (*iter)->length;
          (*iter)->perform_operation();
        }
        operations_in_progress.clear();
      }
      last_iteration_start_time = this_iteration_start_time;
    }

  }

  rate_limiting_group::rate_limiting_group(uint32_t upload_bytes_per_second, uint32_t download_bytes_per_second) :
    my(new detail::rate_limiting_group_impl(upload_bytes_per_second, download_bytes_per_second))
  {
  }

  rate_limiting_group::~rate_limiting_group()
  {
  }

  void rate_limiting_group::set_upload_limit(uint32_t upload_bytes_per_second)
  {
    my->_upload_bytes_per_second = upload_bytes_per_second;
  }

  uint32_t rate_limiting_group::get_upload_limit() const
  {
    return my->_upload_bytes_per_second;
  }

  void rate_limiting_group::set_download_limit(uint32_t download_bytes_per_second)
  {
    my->_download_bytes_per_second = download_bytes_per_second;
  }

  uint32_t rate_limiting_group::get_download_limit() const
  {
    return my->_download_bytes_per_second;
  }

  void rate_limiting_group::add_tcp_socket(tcp_socket* tcp_socket_to_limit)
  {
    tcp_socket_to_limit->set_io_hooks(my.get());
  }

  void rate_limiting_group::remove_tcp_socket(tcp_socket* tcp_socket_to_stop_limiting)
  {
    tcp_socket_to_stop_limiting->set_io_hooks(NULL);
  }


} // namespace fc 
