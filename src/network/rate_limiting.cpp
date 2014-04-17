#include <fc/network/rate_limiting.hpp>
#include <fc/network/tcp_socket.hpp>
#include <fc/network/ip.hpp>
#include <fc/fwd_impl.hpp>
#include <fc/asio.hpp>
#include <fc/log/logger.hpp>
#include <fc/io/stdio.hpp>
#include <fc/exception/exception.hpp>

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

    class rate_limiting_group_impl
    {
    public:
      uint32_t _upload_bytes_per_second;
      uint32_t _download_bytes_per_second;

      microseconds _granularity;

      typedef std::list<std::unique_ptr<rate_limited_operation> > rate_limited_operation_list;
      rate_limited_operation_list _read_operations_in_progress;
      rate_limited_operation_list _read_operations_for_next_iteration;
      rate_limited_operation_list _write_operations_in_progress;
      rate_limited_operation_list _write_operations_for_next_iteration;

      time_point _last_write_iteration_time;

      rate_limiting_group_impl(uint32_t upload_bytes_per_second, uint32_t download_bytes_per_second);

      size_t readsome(boost::asio::ip::tcp::socket& socket, char* buffer, size_t length);
      size_t writesome(boost::asio::ip::tcp::socket& socket, const char* buf, size_t len);

      void process_pending_reads();
      void process_pending_writes();
      void process_pending_operations(rate_limited_operation_list& operations_in_progress,
                                      rate_limited_operation_list& operations_for_next_iteration);
    };

    rate_limiting_group_impl::rate_limiting_group_impl(uint32_t upload_bytes_per_second, uint32_t download_bytes_per_second) :
      _upload_bytes_per_second(upload_bytes_per_second),
      _download_bytes_per_second(download_bytes_per_second),
      _granularity(fc::milliseconds(50))
    {
    }

    size_t rate_limiting_group_impl::readsome(boost::asio::ip::tcp::socket&  socket, char* buffer, size_t length)
    {
      promise<size_t>::ptr completion_promise(new promise<size_t>());
      _read_operations_for_next_iteration.emplace_back(std::make_unique<rate_limited_tcp_read_operation>(socket, buffer, length, completion_promise));
      return completion_promise->wait();
    }
    size_t rate_limiting_group_impl::writesome(boost::asio::ip::tcp::socket&  socket, const char* buffer, size_t length)
    {
      if (_upload_bytes_per_second)
      {
        promise<size_t>::ptr completion_promise(new promise<size_t>());
        _write_operations_for_next_iteration.emplace_back(std::make_unique<rate_limited_tcp_write_operation>(socket, buffer, length, completion_promise));
        return completion_promise->wait();
      }
      else
        return asio::write_some(socket, boost::asio::buffer(buffer, length));
    }
    void rate_limiting_group_impl::process_pending_reads()
    {
      process_pending_operations(_read_operations_in_progress, _read_operations_for_next_iteration);
    }
    void rate_limiting_group_impl::process_pending_writes()
    {
      process_pending_operations(_write_operations_in_progress, _write_operations_for_next_iteration);
    }
    void rate_limiting_group_impl::process_pending_operations(rate_limited_operation_list& operations_in_progress,
                                                              rate_limited_operation_list& operations_for_next_iteration)
    {
      // lock here for multithreaded
      std::copy(std::make_move_iterator(operations_for_next_iteration.begin()),
                std::make_move_iterator(operations_for_next_iteration.end()),
                std::back_inserter(operations_in_progress));
      operations_for_next_iteration.clear();

      // find out how much time since our last write
      time_point this_write_iteration_start_time = time_point::now();
      if (_upload_bytes_per_second) // the we are limiting upload speed
      {
        microseconds time_since_last_iteration = this_write_iteration_start_time - _last_write_iteration_time;
        if (time_since_last_iteration > seconds(1))
          time_since_last_iteration = seconds(1);
        else if (time_since_last_iteration < microseconds(0))
          time_since_last_iteration = microseconds(0);

        uint32_t total_bytes_for_this_iteration = 
          (uint32_t)(time_since_last_iteration.count() / _upload_bytes_per_second / seconds(1).count());
        if (total_bytes_for_this_iteration)
        {
          // sort the pending writes in order of the number of bytes they need to write, smallest first
          std::vector<rate_limited_operation*> operations_sorted_by_length;
          operations_sorted_by_length.reserve(operations_in_progress.size());
          for (std::unique_ptr<rate_limited_operation>& operation_data : operations_in_progress)
            operations_sorted_by_length.push_back(operation_data.get());
          std::sort(operations_sorted_by_length.begin(), operations_sorted_by_length.end(), is_operation_shorter());

          // figure out how many bytes each writer is allowed to write
          uint32_t bytes_remaining_to_allocate = total_bytes_for_this_iteration;
          while (!operations_sorted_by_length.empty())
          {
            uint32_t bytes_permitted_for_this_writer = bytes_remaining_to_allocate / operations_sorted_by_length.size();
            uint32_t bytes_allocated_for_this_writer = std::min(operations_sorted_by_length.back()->length, bytes_permitted_for_this_writer);
            operations_sorted_by_length.back()->permitted_length = bytes_allocated_for_this_writer;
            bytes_remaining_to_allocate -= bytes_allocated_for_this_writer;
            operations_sorted_by_length.pop_back();
          }

          // kick off the writes in first-come order
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
      else // upload speed is unlimited
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
      _last_write_iteration_time = this_write_iteration_start_time;
    }

  }

  rate_limiting_group::rate_limiting_group(uint32_t upload_bytes_per_second, uint32_t download_bytes_per_second) :
    my(new detail::rate_limiting_group_impl(upload_bytes_per_second, download_bytes_per_second))
  {
  }

  rate_limiting_group::~rate_limiting_group()
  {
  }



} // namespace fc 
