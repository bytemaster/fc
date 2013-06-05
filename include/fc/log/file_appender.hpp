#pragma once
#include <fc/log/appender.hpp>
#include <fc/log/logger.hpp>
#include <fc/filesystem.hpp>

namespace fc {

class varaint;

class file_appender : public appender {
    public:
         struct config {
            config( const fc::path& p = "log.txt" );

            fc::string                         format;
            fc::path                           filename;
            bool                               flush;
            bool                               truncate;
         };
         file_appender( const variant& args );
         ~file_appender();
         virtual void log( const log_message& m );

      private:
         class impl;
         fc::shared_ptr<impl> my;
   };
} // namespace fc

#include <fc/reflect/reflect.hpp>
FC_REFLECT( fc::file_appender::config, (format)(filename)(flush)(truncate) )
