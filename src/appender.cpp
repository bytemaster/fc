#include <fc/appender.hpp>
#include <fc/logger.hpp>
#include <fc/value.hpp>
#include <fc/unique_lock.hpp>
#include <unordered_map>
#include <string>
#include <fc/spin_lock.hpp>
#include <fc/scoped_lock.hpp>
#include <fc/console_defines.h>
#include <fc/log.hpp>
#include <fc/value_cast.hpp>
#include <fc/console_appender.hpp>
#include <fc/file_appender.hpp>


namespace fc {

   static fc::spin_lock appender_spinlock;
   std::unordered_map<std::string,appender::ptr>& get_appender_map() {
     static std::unordered_map<std::string,appender::ptr> lm;
     return lm;
   }
   std::unordered_map<std::string,appender_factory::ptr>& get_appender_factory_map() {
     static std::unordered_map<std::string,appender_factory::ptr> lm;
     return lm;
   }
   appender::ptr appender::get( const fc::string& s ) {
      scoped_lock<spin_lock> lock(appender_spinlock);
      return get_appender_map()[s];
   }
   bool  appender::register_appender( const fc::string& type, const appender_factory::ptr& f )
   {
      get_appender_factory_map()[type] = f;
      return true;
   }
   appender::ptr appender::create( const fc::string& name, const fc::string& type, const value& args  )
   {
      auto fact_itr = get_appender_factory_map().find(type);
      if( fact_itr == get_appender_factory_map().end() ) {
         wlog( "Unknown appender type '%s'", type.c_str() );
         return appender::ptr();
      }
      auto ap = fact_itr->second->create( args );
      get_appender_map()[name] = ap;
      return ap;
   }
   
   static bool reg_console_appender = appender::register_appender<console_appender>( "console" );
   static bool reg_file_appender = appender::register_appender<file_appender>( "file" );
} // namespace fc
