#pragma once
#include <fc/logger.hpp>

namespace fc {
   class path;
   struct appender_config {
      appender_config(const fc::string& n="",const fc::string& t="", const value& a=value())
      :name(n),type(t),args(a),enabled(true){}
      string name;
      string type;
      value  args;
      bool   enabled;
   };

   struct logger_config {
      logger_config(const fc::string& n=""):name(n),enabled(true),additivity(false){}
      string                           name;
      ostring                          parent;
      /// if not set, then parents level is used.
      fc::optional<log_level::type>    level;
      bool                             enabled;
      /// if any appenders are sepecified, then parent's appenders are not set.
      bool                             additivity;
      fc::vector<string>               appenders;
   };

   struct logging_config {
      static logging_config default_config();
      fc::vector<string>          includes;
      fc::vector<appender_config> appenders;
      fc::vector<logger_config>   loggers;
   };

   void configure_logging( const fc::path& log_config );
   bool configure_logging( const logging_config& l );
}

#include <fc/reflect.hpp>
FC_REFLECT( fc::appender_config, (name)(type)(args)(enabled) )
FC_REFLECT( fc::logger_config, (name)(parent)(level)(enabled)(additivity)(appenders) )
FC_REFLECT( fc::logging_config, (includes)(appenders)(loggers) )
