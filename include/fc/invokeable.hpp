#ifndef _FC_INVOKEABLE_HPP_
#define _FC_INVOKEABLE_HPP_

namespace fc { 
  
  class invokeable  {
    public:
      virtual ~invokeable(){};

      virtual void invoke( const promise::ptr& prom, const string& name, size_t num_params, reflect::cref* params );
      
      void invoke( const std::string& name ) { invoke( promise::ptr(), name, 0, 0 ); }
  };

}

#endif  // _FC_INVOKEABLE_HPP_
