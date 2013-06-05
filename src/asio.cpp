#include <fc/asio.hpp>
#include <fc/thread/thread.hpp>
#include <boost/thread.hpp>
#include <fc/log/logger.hpp>

namespace fc {
  namespace asio {
    namespace detail {
        void read_write_handler( const promise<size_t>::ptr& p, const boost::system::error_code& ec, size_t bytes_transferred ) {
            if( !ec ) p->set_value(bytes_transferred);
            else {
            //   elog( "%s", boost::system::system_error(ec).what() );
            //   p->set_exception( fc::copy_exception( boost::system::system_error(ec) ) );
                if( ec == boost::asio::error::eof )
                {
                  p->set_exception( fc::exception_ptr( new fc::eof_exception( 
                          FC_LOG_MESSAGE( error, "${message} ", ("message", boost::system::system_error(ec).what())) ) ) );
                }
                else
                {
                 // elog( "${message} ", ("message", boost::system::system_error(ec).what()));
                  p->set_exception( fc::exception_ptr( new fc::exception( 
                          FC_LOG_MESSAGE( error, "${message} ", ("message", boost::system::system_error(ec).what())) ) ) );
                }
            }
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
                //elog( "%s", boost::system::system_error(ec).what() );
                //p->set_exception( fc::copy_exception( boost::system::system_error(ec) ) );
                p->set_exception( 
                    fc::exception_ptr( new fc::exception( 
                        FC_LOG_MESSAGE( error, "process exited with: ${message} ", 
                                        ("message", boost::system::system_error(ec).what())) ) ) );
            }
        }
    }
    boost::asio::io_service& default_io_service(bool cleanup) {
        static boost::asio::io_service       io;
        static boost::asio::io_service::work the_work(io);
        static fc::thread fc1("asio1");
        static fc::thread fc2("asio2");
        static fc::thread fc3("asio3");
        static fc::future<void> future1( fc1.async([=]() { io.run(); }) );
        static fc::future<void> future2( fc2.async([=]() { io.run(); }) );
        static fc::future<void> future3( fc3.async([=]() { io.run(); }) );
        /*
        static boost::thread                 io_t([=] { fc1 = &fc::thread::current(); fc1->set_name("asio1");  io.run(); });
        static boost::thread                 io_t2([=]{ fc2 = &fc::thread::current(); fc2->set_name("asio2");  io.run(); });
        static boost::thread                 io_t3([=]{ fc3 = &fc::thread::current(); fc3->set_name("asio3");  io.run(); });
        */
        if (cleanup)
        {
          io.stop();
          fc1.quit();
          fc2.quit();
          fc3.quit();
          future1.wait();
          future2.wait();
          future3.wait();
        }

        return io;
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
