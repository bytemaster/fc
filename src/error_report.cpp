#include <fc/error_report.hpp>

namespace fc {

error_frame::error_frame( const fc::string& f, uint64_t l, const fc::string& m, const fc::string& d, fc::value met )
:desc(d),file(f),line(l),method(m),meta(fc::move(met)){}

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

} // namespace fc
