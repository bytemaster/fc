#pragma once
#include <fc/appender.hpp>
#include <fc/logger.hpp>
#include <fc/filesystem.hpp>

namespace fc {

class file_appender : public appender {
    public:
         struct config {
            config( const fc::path& p = "log.txt" );

            fc::string                         format;
            fc::path                           filename;
            bool                               flush;
            bool                               truncate;
         };
         file_appender( const value& args );
         ~file_appender();
         virtual void log( const log_message& m );

      private:
         class impl;
         fc::shared_ptr<impl> my;
   };
} // namespace fc

#include <fc/reflect.hpp>
FC_REFLECT( fc::file_appender::config, (format)(filename)(flush)(truncate) )
