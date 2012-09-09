#include <fc/asio.hpp>
#include <fc/thread.hpp>
#include <boost/thread.hpp>

namespace fc {
  namespace asio {
    namespace detail {
        void read_write_handler( const promise<size_t>::ptr& p, const boost::system::error_code& ec, size_t bytes_transferred ) {
            if( !ec ) p->set_value(bytes_transferred);
            else p->set_exception( fc::copy_exception( boost::system::system_error(ec) ) );
        }
        void read_write_handler_ec( promise<size_t>* p, boost::system::error_code* oec, const boost::system::error_code& ec, size_t bytes_transferred ) {
            p->set_value(bytes_transferred);
            *oec = ec;
        }
        void error_handler( const promise<boost::system::error_code>::ptr& p, 
                              const boost::system::error_code& ec ) {
            p->set_value(ec);
        }

        void error_handler_ec( promise<boost::system::error_code>* p, 
                              const boost::system::error_code& ec ) {
            p->set_value(ec);
        }

        template<typename EndpointType, typename IteratorType>
        void resolve_handler( 
                             const typename promise<std::vector<EndpointType> >::ptr& p,
                             const boost::system::error_code& ec, 
                             IteratorType itr) {
            if( !ec ) {
                std::vector<EndpointType> eps;
                while( itr != IteratorType() ) {
                    eps.push_back(*itr);
                    ++itr;
                }
                p->set_value( eps );
            } else {
                p->set_exception( fc::copy_exception( boost::system::system_error(ec) ) );
            }
        }
    }
    boost::asio::io_service& default_io_service() {
        static boost::asio::io_service*      io = new boost::asio::io_service();
        static boost::asio::io_service::work the_work(*io);
        static boost::thread                 io_t([=] { fc::thread::current().set_name("asio1"); io->run(); });
        static boost::thread                 io_t2([=]{ fc::thread::current().set_name("asio2"); io->run(); });
        static boost::thread                 io_t3([=]{ fc::thread::current().set_name("asio3"); io->run(); });
        return *io;
    }

    namespace tcp {
        std::vector<boost::asio::ip::tcp::endpoint> resolve( const std::string& hostname, const std::string& port) {
            resolver res( fc::asio::default_io_service() );
            promise<std::vector<boost::asio::ip::tcp::endpoint> >::ptr p( new promise<std::vector<boost::asio::ip::tcp::endpoint> >() );
            res.async_resolve( boost::asio::ip::tcp::resolver::query(hostname,port), 
                             boost::bind( detail::resolve_handler<boost::asio::ip::tcp::endpoint,resolver_iterator>, p, _1, _2 ) );
            return p->wait();;
        }
    }
    namespace udp {
                std::vector<udp::endpoint> resolve( resolver& r, const std::string& hostname, const std::string& port) {
                resolver res( fc::asio::default_io_service() );
                promise<std::vector<endpoint> >::ptr p( new promise<std::vector<endpoint> >() );
                res.async_resolve( resolver::query(hostname,port), 
                                    boost::bind( detail::resolve_handler<endpoint,resolver_iterator>, p, _1, _2 ) );
                return p->wait();
        }
    }
  
} } // namespace fc::asio
