#include <fc/logger.hpp>
#include <fc/thread.hpp>
#include <fc/spin_lock.hpp>
#include <fc/scoped_lock.hpp>
#include <fc/appender.hpp>
#include <fc/filesystem.hpp>
#include <unordered_map>
#include <string>

// tmp... 
#include <fc/json.hpp>

namespace fc {
    log_message::log_message(){}
    log_message::log_message(log_level::type ll, const string& f, int l, const string& fun, const string& fmt )
    :when( fc::time_point::now() ), level(ll),  thread( fc::thread::current().name() ),file(fc::path(f).filename().generic_string()),line(l),method(fun),format(fmt){}
    
    log_message& log_message::operator()( const fc::string& k, fc::value&& v ) {
       args[k] = fc::move(v);
       return *this;
    }
    log_message& log_message::operator()( fc::value&& v ) {
       args.push_back( fc::move(v) );
       return *this;
    }
    log_message& log_message::operator()( const fc::string& k, const fc::value& v ) {
       args[k] = v;
       return *this;
    }
    log_message& log_message::operator()( const fc::value& v ) {
       args.push_back( v );
       return *this;
    }

    class logger::impl : public fc::retainable {
      public:
         impl()
         :_parent(nullptr),_enabled(true),_level(log_level::warn){}
         fc::string       _name;
         logger           _parent;
         bool             _enabled;
         bool             _additivity;
         log_level::type  _level;

         fc::vector<appender::ptr> _appenders;
    };


    logger::logger()
    :my( new impl() ){}

    logger::logger(std::nullptr_t){}

    logger::logger( const logger& l )
    :my(l.my){}

    logger::logger( logger&& l )
    :my(fc::move(l.my)){}

    logger::~logger(){}

    logger& logger::operator=( const logger& l ){
       my = l.my;
       return *this;
    }
    logger& logger::operator=( logger&& l ){
       fc_swap(my,l.my);
       return *this;
    }
    bool operator==( const logger& l, std::nullptr_t ) { return !l.my; }
    bool operator!=( const logger& l, std::nullptr_t ) { return l.my;  }

    bool logger::is_enabled( log_level::type e )const {
       return e >= my->_level;
    }

    void logger::log( log_message m ) {
       m.context = my->_name;
       for( auto itr = my->_appenders.begin(); itr != my->_appenders.end(); ++itr )
          (*itr)->log( m );

       if( my->_additivity && my->_parent != nullptr) {
          my->_parent.log(m);
       }
    }
    void logger::set_name( const fc::string& n ) { my->_name = n; }
    const fc::string& logger::name()const { return my->_name; }

    std::unordered_map<std::string,logger>& get_logger_map() {
      static std::unordered_map<std::string,logger> lm;
      return lm;
    }

    logger logger::get( const fc::string& s ) {
       static fc::spin_lock logger_spinlock;
       scoped_lock<spin_lock> lock(logger_spinlock);
       return get_logger_map()[s];
    }

    logger  logger::get_parent()const { return my->_parent; }
    logger& logger::set_parent(const logger& p) { my->_parent = p; return *this; }

    log_level::type logger::get_log_level()const { return my->_level; }
    logger& logger::set_log_level(log_level::type ll) { my->_level = ll; return *this; }

    void logger::add_appender( const fc::shared_ptr<appender>& a )
    { my->_appenders.push_back(a); }
    

} // namespace fc
