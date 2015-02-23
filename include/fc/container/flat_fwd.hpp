#pragma once 
namespace boost { namespace container {
   template<typename Key, 
            typename Compare, 
            typename Allocator > 
   class flat_set;


   template<typename Key, 
            typename T, 
            typename Compare, 
            typename Allocator > 
   class flat_map;
} } // boost::container

namespace fc {

   template<typename K, typename V>
   using flat_map = boost::container::flat_map<K,V,std::less<K>,std::allocator<std::pair<K,V>> >;
   template<typename V>
   using flat_set = boost::container::flat_set<V,std::less<V>, std::allocator<V> >;

} // fc
