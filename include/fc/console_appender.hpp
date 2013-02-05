#pragma once
#include <fc/appender.hpp>
#include <fc/logger.hpp>

namespace fc {
   class console_appender : public appender {
       public:
            struct color {
                enum type {
                   red,
                   green,
                   brown,
                   blue,
                   magenta,
                   cyan,
                   white,
                   console_default,
                };
            };
            struct stream { enum type { std_out, std_error }; };

            struct level_color {
               level_color( log_level::type l=log_level::all, 
                            color::type c=color::console_default )
               :level(l),color(c){}

               log_level::type                   level;
               console_appender::color::type     color;
            };

            struct config {
               config()
               :format( "${when} ${thread} ${context} ${file}:${line} ${method} ${level}]  ${message}" ),
                stream(console_appender::stream::std_error),flush(true){}

               fc::string                         format;
               console_appender::stream::type     stream;
               fc::vector<level_color>            level_colors;
               bool                               flush;
            };


            console_appender( const value& args );
            const char* get_color( log_level::type l )const;
            virtual void log( const log_message& m );
       private:
            config                      cfg;
            color::type                 lc[log_level::off+1];
   };
} // namespace fc

#include <fc/reflect.hpp>
FC_REFLECT_ENUM( fc::console_appender::stream::type, (std_out)(std_error) )
FC_REFLECT_ENUM( fc::console_appender::color::type, (red)(green)(brown)(blue)(magenta)(cyan)(white)(console_default) )
FC_REFLECT( fc::console_appender::level_color, (level)(color) )
FC_REFLECT( fc::console_appender::config, (format)(stream)(level_colors)(flush) )
