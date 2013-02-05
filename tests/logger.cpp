#include <fc/logger_config.hpp>

int main( int argc, char** argv ) {
   auto lgr = fc::logger::get();
   fc::configure_logging( fc::logging_config::default_config() );
   fc_dlog( lgr, "Hello Debug" );
   fc_ilog( lgr, "Hello Info" );
   fc_wlog( lgr, "Hello Warn" );
   fc_elog( lgr, "Hello Error" );
   fc_flog( lgr, "Hello Fatal" );
   return 0;
}
