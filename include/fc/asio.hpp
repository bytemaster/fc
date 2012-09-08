/**
 *  @file fc/cmt/asio.hpp
 *  @brief defines wrappers for boost::asio functions
 */
#ifndef _FC_ASIO_HPP_
#define _FC_ASIO_HPP_
#include <boost/asio.hpp>
#include <fc/future.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/combine.hpp>

namespace fc { 
/**
 *  @brief defines fc::cmt wrappers for boost::asio functions.
 */
namespace asio {
    /**
     *  @brief internal implementation types/methods for fc::cmt::asio
     */
    namespace detail {
        using namespace fc;

        void read_write_handler( const promise<size_t>::ptr& p, 
                                 const boost::system::error_code& ec, 
                                size_t bytes_transferred );
        void read_write_handler_ec( promise<size_t>* p, 
                                    boost::system::error_code* oec, 
                                    const boost::system::error_code& ec, 
                                    size_t bytes_transferred );
        void error_handler( const promise<boost::system::error_code>::ptr& p, 
                              const boost::system::error_code& ec );
        void error_handler_ec( promise<boost::system::error_code>* p, 
                              const boost::system::error_code& ec ); 

        template<typename C>
        struct non_blocking { 
          bool operator()( C& c ) { return c.non_blocking(); } 
          bool operator()( C& c, bool s ) { c.non_blocking(s); return true; } 
        };

        #if WIN32  // windows stream handles do not support non blocking!
	       template<>
         struct non_blocking<boost::asio::windows::stream_handle> { 
	          typedef boost::asio::windows::stream_handle C;
            bool operator()( C& ) { return false; } 
            bool operator()( C&, bool ) { return false; } 
        };
        #endif 
    }
    /**
     * @return the default boost::asio::io_service for use with fc::cmt::asio
     * 
     * This IO service is automatically running in its own thread to service asynchronous
     * requests without blocking any other threads.
     */
    boost::asio::io_service& default_io_service();

    /** 
     *  @brief wraps boost::asio::async_read
     *  @pre s.non_blocking() == true
     *  @return the number of bytes read.
     */
    template<typename AsyncReadStream, typename MutableBufferSequence>
    size_t read( AsyncReadStream& s, const MutableBufferSequence& buf ) {
        detail::non_blocking<AsyncReadStream> non_blocking;

        // TODO: determine if non_blocking query results in a system call that
        // will slow down every read... 
        if( non_blocking(s) || non_blocking(s,true) ) {
            boost::system::error_code ec;
            size_t r = boost::asio::read( s, buf, ec );
            if( !ec ) return r;
            if( ec != boost::asio::error::would_block ) 
                  BOOST_THROW_EXCEPTION( boost::system::system_error(ec) );
        } 
        
        promise<size_t>::ptr p(new promise<size_t>("fc::cmt::asio::read"));
        boost::asio::async_read( s, buf, boost::bind( detail::read_write_handler, p, _1, _2 ) );
        return p->wait();
    }
    /** 
     *  This method will read at least 1 byte from the stream and will
     *  cooperatively block until that byte is available or an error occurs.
     *  
     *  If the stream is not in 'non-blocking' mode it will be put in 'non-blocking'
     *  mode it the stream supports s.non_blocking() and s.non_blocking(bool).
     *
     *  If in non blocking mode, the call will be synchronous avoiding heap allocs
     *  and context switching. If the sync call returns 'would block' then an
     *  promise is created and an async read is generated.
     *
     *  @return the number of bytes read.
     */
    template<typename AsyncReadStream, typename MutableBufferSequence>
    size_t read_some( AsyncReadStream& s, const MutableBufferSequence& buf ) {
        detail::non_blocking<AsyncReadStream> non_blocking;

        // TODO: determine if non_blocking query results in a system call that
        // will slow down every read... 
        if( non_blocking(s) || non_blocking(s,true) ) {
            boost::system::error_code ec;
            size_t r = s.read_some( buf, ec );
            if( !ec ) return r;
            if( ec != boost::asio::error::would_block ) 
                  BOOST_THROW_EXCEPTION( boost::system::system_error(ec) );
        }
        
        promise<size_t>::ptr p(new promise<size_t>("fc::cmt::asio::read_some"));
        s.async_read_some( buf, boost::bind( detail::read_write_handler, p, _1, _2 ) );
        return p->wait();
    }

    /** @brief wraps boost::asio::async_write
     *  @return the number of bytes written
     */
    template<typename AsyncWriteStream, typename ConstBufferSequence>
    size_t write( AsyncWriteStream& s, const ConstBufferSequence& buf ) {
        detail::non_blocking<AsyncWriteStream> non_blocking;

        if( non_blocking(s) || non_blocking(s,true) ) {
            boost::system::error_code ec;
            size_t r = boost::asio::write( s, buf, ec );
            if( !ec ) return r;
            if( ec != boost::asio::error::would_block) {
                BOOST_THROW_EXCEPTION( boost::system::system_error(ec) );
            }
        }
        promise<size_t>::ptr p(new promise<size_t>("fc::cmt::asio::write"));
        boost::asio::async_write(s, buf, boost::bind( detail::read_write_handler, p, _1, _2 ) );
        return p->wait();
    }

    /** 
     *  @pre s.non_blocking() == true
     *  @brief wraps boost::asio::async_write_some
     *  @return the number of bytes written
     */
    template<typename AsyncWriteStream, typename ConstBufferSequence>
    size_t write_some( AsyncWriteStream& s, const ConstBufferSequence& buf ) {
        detail::non_blocking<AsyncWriteStream> non_blocking;

        if( non_blocking(s) || non_blocking(s,true) ) {
            boost::system::error_code ec;
            size_t r = s.write_some( buf, ec );
            if( !ec ) return r;
            if( ec != boost::asio::error::would_block) {
                BOOST_THROW_EXCEPTION( boost::system::system_error(ec) );
            }
        }
        promise<size_t>::ptr p(new promise<size_t>("fc::cmt::asio::write_some"));
        s.async_write_some( buf, boost::bind( detail::read_write_handler, p, _1, _2 ) );
        return p->wait();
    }

    template<typename AsyncWriteStream>
    class sink : public boost::iostreams::sink {
      public:
    //     struct category : boost::iostreams::sink::category {};
        typedef char      type;

        sink( AsyncWriteStream& p ):m_stream(p){}
    
        std::streamsize write( const char* s, std::streamsize n ) {
          return fc::cmt::asio::write( m_stream, boost::asio::const_buffers_1(s,n) );
        }
        void close() { m_stream.close(); }
    
      private:
         AsyncWriteStream&      m_stream;
    };

    template<typename AsyncReadStream>
    class source : public boost::iostreams::source {
      public:
        //     struct category : boost::iostreams::sink::category {};
        typedef char      type;

        source( AsyncReadStream& p ):m_stream(p){}
    
        std::streamsize read( char* s, std::streamsize n ) {
          return fc::cmt::asio::read_some( m_stream, boost::asio::buffer(s,n) );
        }
        void close() { m_stream.close(); }
    
      private:
        AsyncReadStream&      m_stream;
    };
    template<typename AsyncStream>
    class io_device {
      public:
        typedef boost::iostreams::bidirectional_device_tag category;
        typedef char                                     char_type;

        io_device( AsyncStream& p ):m_stream(p){}
    
        std::streamsize write( const char* s, std::streamsize n ) {
          return fc::cmt::asio::write( m_stream, boost::asio::const_buffers_1(s,static_cast<size_t>(n)) );
        }
        std::streamsize read( char* s, std::streamsize n ) {
          try {
            return fc::cmt::asio::read_some( m_stream, boost::asio::buffer(s,n) );
          } catch ( const boost::system::system_error& e ) {
            if( e.code() == boost::asio::error::eof )  
                return -1;
            throw;
          }
        }
        void close() { m_stream.close(); }
    
      private:
        AsyncStream&      m_stream;
    };


    namespace tcp {
        typedef boost::asio::ip::tcp::endpoint endpoint;
        typedef boost::asio::ip::tcp::resolver::iterator resolver_iterator;
        typedef boost::asio::ip::tcp::resolver resolver;
        std::vector<endpoint> resolve( const std::string& hostname, const std::string& port );

        /** @brief wraps boost::asio::async_accept
          * @post sock is connected
          * @post sock.non_blocking() == true  
          * @throw on error.
          */
        template<typename SocketType, typename AcceptorType>
        void accept( AcceptorType& acc, SocketType& sock ) {
            promise<boost::system::error_code>::ptr p( new promise<boost::system::error_code>("fc::cmt::asio::tcp::accept") );
            acc.async_accept( sock, boost::bind( fc::cmt::asio::detail::error_handler, p, _1 ) );
            auto ec = p->wait();
            if( !ec ) sock.non_blocking(true);
            if( ec ) BOOST_THROW_EXCEPTION( boost::system::system_error(ec) );
        }

        /** @brief wraps boost::asio::socket::async_connect
          * @post sock.non_blocking() == true  
          * @throw on error
          */
        template<typename AsyncSocket, typename EndpointType>
        void connect( AsyncSocket& sock, const EndpointType& ep ) {
            promise<boost::system::error_code>::ptr p(new promise<boost::system::error_code>("fc::cmt::asio::tcp::connect"));
            sock.async_connect( ep, boost::bind( fc::cmt::asio::detail::error_handler, p, _1 ) );
            auto ec = p->wait();
            if( !ec ) sock.non_blocking(true);
            if( ec ) BOOST_THROW_EXCEPTION( boost::system::system_error(ec) );
        }
      
        typedef boost::iostreams::stream<fc::cmt::asio::sink<boost::asio::ip::tcp::socket> >      ostream;
        typedef boost::iostreams::stream<fc::cmt::asio::source<boost::asio::ip::tcp::socket> >    istream;
        typedef boost::iostreams::stream<fc::cmt::asio::io_device<boost::asio::ip::tcp::socket> > iostream;

    }
    namespace udp {
        typedef boost::asio::ip::udp::endpoint endpoint;
        typedef boost::asio::ip::udp::resolver::iterator resolver_iterator;
        typedef boost::asio::ip::udp::resolver resolver;
        /// @brief resolve all udp::endpoints for hostname:port
        std::vector<endpoint> resolve( resolver& r, const std::string& hostname, const std::string& port );
    }


} } // namespace fc::asio

#endif // _BOOST_CMT_ASIO_HPP_
