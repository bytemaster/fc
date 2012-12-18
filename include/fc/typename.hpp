#pragma once
#include <fc/string.hpp>
namespace fc {
  class value;
  template<typename T> class get_typename{};
  template<> struct get_typename<int32_t>  { static const char* name()  { return "int32_t";  } };
  template<> struct get_typename<int64_t>  { static const char* name()  { return "int64_t";  } };
  template<> struct get_typename<int16_t>  { static const char* name()  { return "int16_t";  } };
  template<> struct get_typename<int8_t>   { static const char* name()  { return "int8_t";   } };
  template<> struct get_typename<uint32_t> { static const char* name()  { return "uint32_t"; } };
  template<> struct get_typename<uint64_t> { static const char* name()  { return "uint64_t"; } };
  template<> struct get_typename<uint16_t> { static const char* name()  { return "uint16_t"; } };
  template<> struct get_typename<uint8_t>  { static const char* name()  { return "uint8_t";  } };
  template<> struct get_typename<double>   { static const char* name()  { return "double";   } };
  template<> struct get_typename<float>    { static const char* name()  { return "float";    } };
  template<> struct get_typename<bool>     { static const char* name()  { return "bool";     } };
  template<> struct get_typename<char>     { static const char* name()  { return "char";     } };
  template<> struct get_typename<void>     { static const char* name()  { return "char";     } };
  template<> struct get_typename<string>   { static const char* name()  { return "string";   } };
  template<> struct get_typename<value>   { static const char* name()   { return "value";   } };
}
