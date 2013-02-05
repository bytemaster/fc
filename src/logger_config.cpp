#include <fc/logger_config.hpp>
#include <fc/appender.hpp>
#include <fc/json.hpp>
#include <fc/filesystem.hpp>
#include <unordered_map>
#include <string>

namespace fc {
   std::unordered_map<std::string,logger>& get_logger_map();
   std::unordered_map<std::string,appender::ptr>& get_appender_map();
   logger_config& logger_config::add_appender( const string& s ) { appenders.push_back(s); return *this; }

   void configure_logging( const fc::path& lc )
   {
      configure_logging( fc::json::from_file<logging_config>(lc) );
   }
   bool configure_logging( const logging_config& cfg )
   {
      get_logger_map().clear();
      get_appender_map().clear();

      slog( "\n%s", fc::json::to_pretty_string(cfg).c_str() );
      for( size_t i = 0; i < cfg.appenders.size(); ++i ) {
         appender::create( cfg.appenders[i].name, cfg.appenders[i].type, cfg.appenders[i].args );
        // TODO... process enabled
      }
      for( size_t i = 0; i < cfg.loggers.size(); ++i ) {
         auto lgr = logger::get( cfg.loggers[i].name );

         // TODO: finish configure logger here...
         if( cfg.loggers[i].parent ) {
            lgr.set_parent( logger::get( *cfg.loggers[i].parent ) );
         }
         lgr.set_name(cfg.loggers[i].name);
         lgr.set_log_level( *cfg.loggers[i].level );
         

         for( auto a = cfg.loggers[i].appenders.begin(); a != cfg.loggers[i].appenders.end(); ++a ){
            auto ap = appender::get( *a );
            if( ap ) { lgr.add_appender(ap); }
         }
      }
      return true;
   }

   logging_config logging_config::default_config() {
      slog( "default cfg" );
      logging_config cfg;
      cfg.appenders.push_back( 
             appender_config( "stderr", "console", 
                 fc::value()
                 .set( "stream","std_error")
                 .set( "level_colors", 
                       fc::value().push_back(  fc::value().set( "level","debug").set("color", "green") ) 
                                  .push_back(  fc::value().set( "level","warn").set("color", "brown") ) 
                                  .push_back(  fc::value().set( "level","error").set("color", "red") ) 
                                  .push_back(  fc::value().set( "level","fatal").set("color", "red") ) 
                     )
                 ) ); 
      cfg.appenders.push_back( 
             appender_config( "stdout", "console", 
                 fc::value()
                 .set( "stream","std_out") 
                 .set( "level_colors", 
                       fc::value().push_back(  fc::value().set( "level","debug").set("color", "green") ) 
                                  .push_back(  fc::value().set( "level","warn").set("color", "brown") ) 
                                  .push_back(  fc::value().set( "level","error").set("color", "red") ) 
                                  .push_back(  fc::value().set( "level","fatal").set("color", "red") ) 
                     )
                 ) ); 
      
      logger_config dlc;
      dlc.name = "default";
      dlc.level = log_level::debug;
      dlc.appenders.push_back("stderr");
      cfg.loggers.push_back( dlc );
      return cfg;
   }
   static bool do_default_config      = configure_logging( logging_config::default_config() );
}
