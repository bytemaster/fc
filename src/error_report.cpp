#include <fc/error_report.hpp>
#include <fc/filesystem.hpp>
#include <fc/sstream.hpp>
#include <fc/value.hpp>
#include <fc/json.hpp>
#include <boost/exception_ptr.hpp>
namespace fc {

error_frame::error_frame( const fc::string& f, uint64_t l, const fc::string& m, const fc::string& d, fc::value met )
:desc(d),file(fc::path(f).filename().generic_string()),line(l),method(m),meta(fc::move(met)){}

error_report::error_report()
{
}
error_frame::error_frame(const fc::error_frame& e)
:desc(e.desc),file(e.file),line(e.line),method(e.method),time(e.time),meta(e.meta){}

error_frame::error_frame(fc::error_frame&& e)
:desc(fc::move(e.desc)),
 file(fc::move(e.file)),
 line(e.line),
 method(fc::move(e.method)),
 time(e.time),
 meta(fc::move(e.meta))
  {}

fc::error_frame& fc::error_frame::operator=(const fc::error_frame& f ) {
    auto tmp = f;
    fc_swap( tmp, *this );
    return *this;
}
fc::error_frame& fc::error_frame::operator=(fc::error_frame&& e )
{
    desc=fc::move(e.desc);
    file=fc::move(e.file);
    line=fc::move(e.line);
    method=fc::move(e.method);
    time=e.time;
    meta=fc::move(e.meta);
    return *this;
}

error_report::error_report( const fc::string& file, uint64_t line, const fc::string& method, const fc::string& desc, fc::value meta  )
{
   push_frame( file, line, method, desc, meta );
}


fc::error_frame&  error_report::current()
{
    if( !stack.size() ) stack.resize(1);
    return stack.back();
}

fc::error_report&         error_report::pop_frame()
{
   stack.pop_back();
   return *this;
}

fc::error_report& error_report::push_frame( const fc::string& file, uint64_t line, const fc::string& method, const fc::string& desc, fc::value meta  )
{
   stack.push_back( fc::error_frame( file, line, method, desc, meta ) );
   return *this;
}

fc::error_report&         error_report::append( const error_report& e )
{
   // TODO: what to do about the 'failure...?'
   stack.reserve( stack.size()+e.stack.size());
   for( uint32_t i = 0; i < e.stack.size(); ++i ) {
      stack.push_back( e.stack[i] );
   }
   return *this;
}

fc::string error_frame::to_detail_string()const {
    fc::stringstream ss;
    ss << to_string() << "\n\t";
    ss << file << ":" << line << "\t"<<method;
    if( meta ) ss << "\t" <<fc::json::to_string(*meta);
    return ss.str();
}
fc::string error_frame::to_string()const {
   return substitute( desc, meta ? *meta : fc::value() );
}
#if 0
    fc::stringstream ss;
    size_t prev = 0;
    auto next = desc.find( '$' );
    while( prev != size_t(fc::string::npos) && prev < size_t(desc.size()) ) {
   //   slog( "prev: %d next %d   %s", prev, next, desc.substr(prev,next).c_str() );
      // print everything from the last pos until the first '$'
      ss << desc.substr( prev, size_t(next-prev) );

      // if we got to the end, return it.
      if( next == string::npos ) { return ss.str(); }

      // if we are not at the end, then update the start
      prev = next + 1;

      if( desc[prev] == '{' ) { 
         // if the next char is a open, then find close
          next = desc.find( '}', prev );
          // if we found close... 
          if( next != fc::string::npos ) {
            // the key is between prev and next
            fc::string key = desc.substr( prev+1, (next-prev-1) );
            //slog( "key '%s'", key.c_str() );
            if( meta ) {
                auto itr = meta->find( key.c_str() );
                if( itr != meta->end() ) {
		   if( itr->val.is_string() ) {
		     ss<<itr->val.cast<fc::string>();
		   } else {
                     ss << fc::json::to_string( itr->val );
		   }
                } else {
                   ss << "???";
                }
            }
            prev = next + 1;
            // find the next $
            next = desc.find( '$', prev );
          } else {
            // we didn't find it.. continue to while...
          }
      } else  {
         ss << desc[prev];
         ++prev;
         next = desc.find( '$', prev );
      }
    }
    return ss.str();
}
#endif

fc::string substitute( const fc::string& format, const fc::value& keys ) {
    fc::stringstream ss;
    size_t prev = 0;
    auto next = format.find( '$' );
    while( prev != size_t(fc::string::npos) && prev < size_t(format.size()) ) {
   //   slog( "prev: %d next %d   %s", prev, next, format.substr(prev,next).c_str() );
      // print everything from the last pos until the first '$'
      ss << format.substr( prev, size_t(next-prev) );

      // if we got to the end, return it.
      if( next == string::npos ) { return ss.str(); }

      // if we are not at the end, then update the start
      prev = next + 1;

      if( format[prev] == '{' ) { 
         // if the next char is a open, then find close
          next = format.find( '}', prev );
          // if we found close... 
          if( next != fc::string::npos ) {
            // the key is between prev and next
            fc::string key = format.substr( prev+1, (next-prev-1) );
            //slog( "key '%s'", key.c_str() );
          //  if( keys ) {
                auto itr = keys.find( key.c_str() );
                if( itr != keys.end() ) {
		               if( itr->val.is_string() ) {
		                 ss<<itr->val.cast<fc::string>();
		               } else {
                                 ss << fc::json::to_string( itr->val );
		               }
                } else {
                   ss << "???";
                }
          //  }
            prev = next + 1;
            // find the next $
            next = format.find( '$', prev );
          } else {
            // we didn't find it.. continue to while...
          }
      } else  {
         ss << format[prev];
         ++prev;
         next = format.find( '$', prev );
      }
    }
    return ss.str();
}


fc::string error_report::to_string()const {
  fc::stringstream ss;
  for( uint32_t i = 0; i < stack.size(); ++i ) {
    ss << stack[i].to_string() << "\n";
  }
  return ss.str();
}
fc::string error_report::to_detail_string()const {
  fc::stringstream ss;
  for( uint32_t i = 0; i < stack.size(); ++i ) {
    ss << stack[i].to_detail_string() << "\n";
  }
  return ss.str();
}
fc::exception_ptr error_report::copy_exception() {
   return boost::copy_exception(*this);
}

} // namespace fc
