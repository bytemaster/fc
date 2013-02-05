#include <fc/logger_config.hpp>
#include <fc/file_appender.hpp>
#include <fc/value_cast.hpp>

int main( int argc, char** argv ) {
   auto lgr = fc::logger::get();
   auto dconfig = fc::logging_config::default_config();
   dconfig.appenders.push_back( fc::appender_config("logfile", "file", fc::value(fc::file_appender::config("test.log")) ) );
   dconfig.loggers.push_back( fc::logger_config("main").add_appender("stderr").add_appender("logfile") );
   fc::configure_logging( dconfig );
   fc_dlog( lgr, "Hello Debug" );
   fc_ilog( lgr, "Hello Info" );
   fc_wlog( lgr, "Hello Warn" );
   fc_elog( lgr, "Hello Error" );
   fc_flog( lgr, "Hello Fatal" );


   auto main_lgr = fc::logger::get( "main" );
   fc_dlog( main_lgr, "Hello Debug" );
   fc_ilog( main_lgr, "Hello Info" );
   fc_wlog( main_lgr, "Hello Warn" );
   fc_elog( main_lgr, "Hello Error" );
   fc_flog( main_lgr, "Hello Fatal" );
   return 0;
}
