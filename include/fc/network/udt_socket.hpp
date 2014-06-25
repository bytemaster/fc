#pragma once
#include <fc/utility.hpp>
#include <fc/fwd.hpp>
#include <fc/io/iostream.hpp>
#include <fc/time.hpp>

namespace fc {
   namespace ip { class endpoint; }

   class udt_socket : public virtual iostream 
   {
     public:
       udt_socket();
       ~udt_socket();
   
       void             connect_to( const fc::ip::endpoint& remote_endpoint );

       fc::ip::endpoint remote_endpoint() const;
       fc::ip::endpoint local_endpoint() const;
   
       void get( char& c )
       {
           read( &c, 1 );
       }
   
   
       /// istream interface
       /// @{
       virtual size_t   readsome( char* buffer, size_t max );
       virtual bool     eof()const;
       /// @}
   
       /// ostream interface
       /// @{
       virtual size_t   writesome( const char* buffer, size_t len );
       virtual void     flush();
       virtual void     close();
       /// @}
   
       void open();
       bool is_open()const;
   
     private:
       friend class udt_server;
       int  _udt_socket_id;
   };
   typedef std::shared_ptr<udt_socket> udt_socket_ptr;

} // fc
