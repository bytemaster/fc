#pragma once
#include <fc/io/varint.hpp>
#include <fc/array.hpp>
#include <vector>
#include <string>
#include <unordered_set>
#include <set>

namespace fc { 
   class time_point;
   class time_point_sec;
   class variant;
   class variant_object;
   template<typename IntType, typename EnumType> class enum_type;

   namespace ecc { class public_key; class private_key; }
   namespace raw {
    template<typename Stream, typename IntType, typename EnumType>
    inline void pack( Stream& s, const fc::enum_type<IntType,EnumType>& tp );
    template<typename Stream, typename IntType, typename EnumType>
    inline void unpack( Stream& s, fc::enum_type<IntType,EnumType>& tp );

    template<typename Stream, typename T> inline void pack( Stream& s, const std::set<T>& value );
    template<typename Stream, typename T> inline void unpack( Stream& s, std::set<T>& value );
    template<typename Stream, typename T> inline void pack( Stream& s, const std::unordered_set<T>& value );
    template<typename Stream, typename T> inline void unpack( Stream& s, std::unordered_set<T>& value );
    template<typename Stream> inline void pack( Stream& s, const variant_object& v );
    template<typename Stream> inline void unpack( Stream& s, variant_object& v );
    template<typename Stream> inline void pack( Stream& s, const variant& v );
    template<typename Stream> inline void unpack( Stream& s, variant& v );


    template<typename Stream, typename T> void unpack( Stream& s, fc::optional<T>& v ); 
    template<typename Stream, typename T> void pack( Stream& s, const fc::optional<T>& v );

    template<typename Stream> void unpack( Stream& s, time_point& ); 
    template<typename Stream> void pack( Stream& s, const time_point& );
    template<typename Stream> void unpack( Stream& s, time_point_sec& ); 
    template<typename Stream> void pack( Stream& s, const time_point_sec& );
    template<typename Stream> void unpack( Stream& s, std::string& ); 
    template<typename Stream> void pack( Stream& s, const std::string& );
    template<typename Stream> void unpack( Stream& s, fc::ecc::public_key& ); 
    template<typename Stream> void pack( Stream& s, const fc::ecc::public_key& );
    template<typename Stream> void unpack( Stream& s, fc::ecc::private_key& ); 
    template<typename Stream> void pack( Stream& s, const fc::ecc::private_key& );

    template<typename Stream, typename T> inline void pack( Stream& s, const T& v ); 
    template<typename Stream, typename T> inline void unpack( Stream& s, T& v );

    template<typename Stream, typename T> inline void pack( Stream& s, const std::vector<T>& v );
    template<typename Stream, typename T> inline void unpack( Stream& s, std::vector<T>& v );

    template<typename Stream> inline void pack( Stream& s, const signed_int& v );
    template<typename Stream> inline void unpack( Stream& s, signed_int& vi );

    template<typename Stream> inline void pack( Stream& s, const unsigned_int& v );
    template<typename Stream> inline void unpack( Stream& s, unsigned_int& vi );

    template<typename Stream> inline void pack( Stream& s, const char* v );
    template<typename Stream> inline void pack( Stream& s, const std::vector<char>& value );
    template<typename Stream> inline void unpack( Stream& s, std::vector<char>& value );

    template<typename Stream, typename T, size_t N> inline void pack( Stream& s, const fc::array<T,N>& v);
    template<typename Stream, typename T, size_t N> inline void unpack( Stream& s, fc::array<T,N>& v);

    template<typename Stream> inline void pack( Stream& s, const bool& v );
    template<typename Stream> inline void unpack( Stream& s, bool& v );

    template<typename T> inline std::vector<char> pack( const T& v );
    template<typename T> inline T unpack( const std::vector<char>& s );
    template<typename T> inline T unpack( const char* d, uint32_t s );
    template<typename T> inline void unpack( const char* d, uint32_t s, T& v );
} }
