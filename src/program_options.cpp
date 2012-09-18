#include <fc/program_options.hpp>
#include <fc/fwd_impl.hpp>
#include <sstream>
#include <boost/program_options.hpp>
#include <fc/stream.hpp>

namespace fc { namespace program_options {

  class options_description::impl {
    public:
      impl( const char* c ) 
      :opts(c){}

      boost::program_options::options_description opts;
  };

  options_description::options_description( const char* c )
  :my(c)
  { }

  options_description::~options_description(){
    
  }

  
  options_description& options_description::add_options(){
    return *this; 
  }

  options_description& options_description::operator()( const char* o, const char* desc ){
    my->opts.add_options()( o, desc ); 
    return *this; 
  }

  options_description& options_description::operator()( const char* o, const value<fc::string>& v,               const char* desc ){
    my->opts.add_options()( o, boost::program_options::value<std::string>(reinterpret_cast<std::string*>(v.get())), desc ); 
    return *this; 
  }

  options_description& options_description::operator()( const char* o, const value<uint16_t>& v,                 const char* desc ){
    my->opts.add_options()( o, boost::program_options::value<uint16_t>(v.get()), desc ); 
    return *this; 
  }

  options_description& options_description::operator()( const char* o, const value<std::vector<std::string> >& v, const char* desc ){
    my->opts.add_options()( o, boost::program_options::value<std::vector<std::string> >(v.get()), desc ); 
    //my->opts.add_options()( o, desc ); 
    return *this; 
  }

  class variables_map::impl {
    public:
      boost::program_options::variables_map vm;
  };

  variables_map::variables_map(){}
  variables_map::~variables_map(){}

   void variables_map::parse_command_line( int argc, char** argv, const options_description& d ) {
     boost::program_options::store( boost::program_options::parse_command_line( argc, argv, d.my->opts ), my->vm );
   }
   int variables_map::count( const char* opt ) {
      return my->vm.count(opt);
   }

   fc::ostream& operator<<( fc::ostream& o, const options_description& od ) {
       std::stringstream ss; ss << od.my->opts;
       fc::cout << ss.str().c_str();
       return o;
   }

} } 
