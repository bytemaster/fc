#include <fc/network/udp_socket.hpp>
#include <fc/network/ip.hpp>
#include <fc/network/resolve.hpp>
#include <fc/exception/exception.hpp>
#include <fc/log/gelf_appender.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/thread/scoped_lock.hpp>
#include <fc/thread/thread.hpp>
#include <fc/variant.hpp>
#include <fc/io/json.hpp>

#include <boost/thread/mutex.hpp>
#include <boost/lexical_cast.hpp>
#include <iomanip>
#include <queue>
#include <sstream>

namespace fc 
{

  class gelf_appender::impl : public retainable
  {
  public:
    config                     cfg;
    optional<ip::endpoint>     gelf_endpoint;
    udp_socket                 gelf_socket;
    boost::mutex               socket_mutex;

    impl(const config& c) : 
      cfg(c)
    {
    }

    ~impl()
    {
    }
  };

  gelf_appender::gelf_appender(const variant& args) :
    my(new impl(args.as<config>()))
  {
    try
    {
      try
      {
        // if it's a numeric address:port, this will parse it
        my->gelf_endpoint = ip::endpoint::from_string(my->cfg.endpoint);
      }
      catch (...)
      {
      }
      if (!my->gelf_endpoint)
      {
        // couldn't parse as a numeric ip address, try resolving as a DNS name.  
        // This can yield, so don't do it in the catch block above
        string::size_type colon_pos = my->cfg.endpoint.find(':');
        try
        {
          uint16_t port = boost::lexical_cast<uint16_t>(my->cfg.endpoint.substr(colon_pos + 1, my->cfg.endpoint.size()));

          string hostname = my->cfg.endpoint.substr( 0, colon_pos );
          std::vector<ip::endpoint> endpoints = resolve(hostname, port);
          if (endpoints.empty())
              FC_THROW_EXCEPTION(unknown_host_exception, "The host name can not be resolved: ${hostname}", 
                                 ("hostname", hostname));
          my->gelf_endpoint = endpoints.back();
        }
        catch (const boost::bad_lexical_cast&)
        {
          FC_THROW("Bad port: ${port}", ("port", my->cfg.endpoint.substr(colon_pos + 1, my->cfg.endpoint.size())));
        }
      }

      if (my->gelf_endpoint)
        my->gelf_socket.open();
    }
    catch (...)
    {
      std::cerr << "error opening GELF socket to endpoint ${endpoint}" << my->cfg.endpoint << "\n";
    }
  }

  gelf_appender::~gelf_appender()
  {}

  void gelf_appender::log(const log_message& message)
  {
    if (!my->gelf_endpoint)
      return;

    log_context context = message.get_context();

    mutable_variant_object gelf_message;
    gelf_message["version"] = "1.1";
    gelf_message["host"] = my->cfg.host;
    gelf_message["short_message"] = format_string(message.get_format(), message.get_data());
    
    gelf_message["timestamp"] = context.get_timestamp().time_since_epoch().count() / 1000000.;

    switch (context.get_log_level())
    {
    case log_level::debug:
      gelf_message["level"] = 7; // debug
      break;
    case log_level::info:
      gelf_message["level"] = 6; // info
      break;
    case log_level::warn:
      gelf_message["level"] = 4; // warning
      break;
    case log_level::error:
      gelf_message["level"] = 3; // error
      break;
    case log_level::all:
    case log_level::off:
      // these shouldn't be used in log messages, but do something deterministic just in case
      gelf_message["level"] = 6; // info
      break;
    }

    if (!context.get_context().empty())
      gelf_message["context"] = context.get_context();
    gelf_message["_line"] = context.get_line_number();
    gelf_message["_file"] = context.get_file();
    gelf_message["_method_name"] = context.get_method();
    gelf_message["_thread_name"] = context.get_thread_name();
    if (!context.get_task_name().empty())
      gelf_message["_task_name"] = context.get_task_name();

    string gelf_message_as_string = json::to_string(gelf_message);
    std::shared_ptr<char> send_buffer(new char[gelf_message_as_string.size()], [](char* p){ delete[] p; });
    memcpy(send_buffer.get(), gelf_message_as_string.c_str(), gelf_message_as_string.size());

    {
      scoped_lock<boost::mutex> lock(my->socket_mutex);
      my->gelf_socket.send_to(send_buffer, gelf_message_as_string.size(), *my->gelf_endpoint);
    }
  }
} // fc
