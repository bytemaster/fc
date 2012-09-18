#include <fc/interprocess/file_mapping.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <fc/fwd_impl.hpp>

namespace fc {
  file_mapping::file_mapping( const char* file, mode_t m )
  :my(file, m == read_only ? boost::interprocess::read_only : boost::interprocess::read_write ){}
  file_mapping::~file_mapping(){}



  mapped_region::mapped_region( const file_mapping& fm, mode_t m, size_t start, size_t size )
  :my( *fm.my, m == read_only ? boost::interprocess::read_only : boost::interprocess::read_write  ,start, size) { }
  mapped_region::mapped_region( const file_mapping& fm, mode_t m )
  :my( *fm.my, m == read_only ? boost::interprocess::read_only : boost::interprocess::read_write) { }
  mapped_region::~mapped_region(){}
  void* mapped_region::get_address()const { return my->get_address(); }
  size_t mapped_region::get_size()const { return my->get_size();    }
}
