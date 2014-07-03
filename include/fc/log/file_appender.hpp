#pragma once

#include <fc/filesystem.hpp>
#include <fc/log/appender.hpp>
#include <fc/log/logger.hpp>
#include <fc/time.hpp>

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
            bool                               rotate;
            microseconds                       rotation_interval;
            microseconds                       rotation_limit;
            bool                               rotation_compression;
         };
         file_appender( const variant& args );
         ~file_appender();
         virtual void log( const log_message& m )override;

      private:
         class impl;
         fc::shared_ptr<impl> my;
   };
} // namespace fc

#include <fc/reflect/reflect.hpp>
FC_REFLECT( fc::file_appender::config,
            (format)(filename)(flush)(truncate)(rotate)(rotation_interval)(rotation_limit)(rotation_compression) )
