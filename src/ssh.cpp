#include <fc/ssh/client.hpp>
#include <fc/exception.hpp>
#include <fc/iostream.hpp>
#include <fc/log.hpp>
#include <fc/thread.hpp>
#include <fc/vector.hpp>
#include <fc/interprocess/file_mapping.hpp>
#include <fc/unique_lock.hpp>
#include <fc/mutex.hpp>
#include <libssh2.h>
#include <libssh2_sftp.h>
#include <memory>
#include <fc/asio.hpp>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

namespace fc { namespace ssh {
  
  namespace detail {
    static int ssh_init = libssh2_init(0);

    class process_impl : public fc::retainable {
      public:
        process_impl( const client& c ):sshc(c){}

        client  sshc;
    };



    class client_impl : public fc::retainable {
      public:
        LIBSSH2_SESSION*            session;
        LIBSSH2_KNOWNHOSTS*         knownhosts;
        LIBSSH2_SFTP*               sftp;

        std::unique_ptr<boost::asio::ip::tcp::socket> sock;
        boost::asio::ip::tcp::endpoint                endpt;

        fc::mutex                   scp_send_mutex;
        fc::string                  uname;
        fc::string                  upass;
        fc::string                  pubkey;
        fc::string                  privkey;
        fc::string                  passphrase;
        fc::string                  hostname;
        uint16_t                    port;
        bool                        session_connected;
        fc::promise<boost::system::error_code>::ptr      read_prom;
        fc::promise<boost::system::error_code>::ptr      write_prom;

        static void kbd_callback(const char *name, int name_len, 
                     const char *instruction, int instruction_len, int num_prompts,
                     const LIBSSH2_USERAUTH_KBDINT_PROMPT *prompts,
                     LIBSSH2_USERAUTH_KBDINT_RESPONSE *responses,
                     void **abstract)
        {
                int i;
                size_t n;
                char buf[1024];
                client_impl* self = (client_impl*)*abstract;
        
        //        printf("Performing keyboard-interactive authentication.\n");
        
         //       printf("Authentication name: '");
          //      fwrite(name, 1, name_len, stdout);
          //      printf("'\n");
        
           //     printf("Authentication instruction: '");
           //     fwrite(instruction, 1, instruction_len, stdout);
           //     printf("'\n");
        
           //     printf("Number of prompts: %d\n\n", num_prompts);
        
                for (i = 0; i < num_prompts; i++) {
            //        printf("Prompt %d from server: '", i);
                    fwrite(prompts[i].text, 1, prompts[i].length, stdout);
              //      printf("'\n");
        
             //       printf("Please type response: ");
        
                    if( self->upass.size() == 0 ) {
                        fgets(buf, sizeof(buf), stdin);
                        n = strlen(buf);
                        while (n > 0 && strchr("\r\n", buf[n - 1]))
                          n--;
                        buf[n] = 0;
        
                        #ifdef WIN32 // fix warning
                          #define strdup _strdup
                        #endif
                        responses[i].text = strdup(buf);
                        responses[i].length = n;
                    } else {
                        responses[i].text = strdup(self->upass.c_str());
                        responses[i].length = self->upass.size();
                    }
        
                 //   printf("Response %d from user is '", i);
                 //   fwrite(responses[i].text, 1, responses[i].length, stdout);
                 //   printf("'\n\n");
                }
        
             //   printf("Done. Sending keyboard-interactive responses to server now.\n");
        } // kbd_callback

        void connect() {
          try {
            if( libssh2_init(0) < 0  ) { FC_THROW_MSG( "Unable to init libssh2" ); }
           
            auto eps = fc::asio::tcp::resolve( hostname, fc::lexical_cast<fc::string>(port) );
            if( eps.size() == 0 ) {
               FC_THROW_MSG( "Unable to resolve '%s'", hostname );
            }
            sock.reset( new boost::asio::ip::tcp::socket( fc::asio::default_io_service() ) );
            
            for( uint32_t i = 0; i < eps.size(); ++i ) {
               try {
                 fc::asio::tcp::connect( *sock, eps[i] );
                 endpt = eps[i];
                 break;
               } catch ( ... ) {}
            }
            session = libssh2_session_init(); 
            *libssh2_session_abstract(session) = this;
            
            libssh2_session_set_blocking( session, 0 );
            int ec = libssh2_session_handshake( session, sock->native() );
            while( ec == LIBSSH2_ERROR_EAGAIN ) {
              wait_on_socket();
              ec = libssh2_session_handshake( session, sock->native() );
            }
            if( ec < 0 ) {
              char* msg;
              libssh2_session_last_error( session, &msg, 0, 0 );
              FC_THROW_MSG( "Handshake error: %s - %s", ec, msg );
            }
            /*const char* fingerprint = */libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA1);
             
            // try to authenticate, throw on error.
            authenticate();
          } catch (...) {
             close();
             throw;
          }
        }

        void close() {
            if( session ) {
               if( sftp ) {
                 int ec = libssh2_sftp_shutdown(sftp);
                 try {
                     while( ec == LIBSSH2_ERROR_EAGAIN ) {
                        wait_on_socket();
                        ec = libssh2_sftp_shutdown(sftp);
                     } 
                 }catch(...){
                  elog( "... caught error closing sftp session???" );
                 }
                 sftp = 0;
               }
               try {
                 int ec = libssh2_session_disconnect(session, "exit cleanly" );
                 while( ec == LIBSSH2_ERROR_EAGAIN ) {
                    wait_on_socket();
                    ec = libssh2_session_disconnect(session, "exit cleanly" );
                 }
                 ec = libssh2_session_free(session);
                 while( ec == LIBSSH2_ERROR_EAGAIN ) {
                    wait_on_socket();
                    ec = libssh2_session_free(session );
                 }
                 session = 0;
               } catch ( ... ){
                  elog( "... caught error freeing session???" );
                  session = 0;
               }
               try {
                 if( sock ) {
                   slog( "closing socket" );
                   sock->close();
                 }
               } catch ( ... ){
                  elog( "... caught error closing socket???" );
               }
               sock.reset(0);
               try {
                if( read_prom ) read_prom->wait();
               } catch ( ... ){
                wlog( "caught error waiting on read prom" );
               }
               try {
                if( write_prom ) write_prom->wait();
               } catch ( ... ){
                wlog( "caught error waiting on write prom" );
               }
            }
        }

        void authenticate() {
            char * alist = libssh2_userauth_list(session, uname.c_str(),uname.size());
            char * msg   = 0;
            int    ec    = 0;

            if(alist==NULL){
                if(libssh2_userauth_authenticated(session)){
                    return; // CONNECTED!
                } 
                ec = libssh2_session_last_error(session,&msg,NULL,0);

                while( !alist && (ec == LIBSSH2_ERROR_EAGAIN) ) {
                    wait_on_socket();
                    alist = libssh2_userauth_list(session, uname.c_str(), uname.size());
                    ec = libssh2_session_last_error(session,&msg,NULL,0);
                }
                if( !alist ) {
                    FC_THROW_MSG( "Error getting authorization list: %s - %s", ec, msg );
                }
            }

            std::vector<std::string> split_alist;
            bool pubkey = false;
            bool pass   = false;
            bool keybd   = false;
            boost::split( split_alist, alist, boost::is_any_of(",") );
            std::for_each( split_alist.begin(), split_alist.end(), [&](const std::string& s){
              if( s == "publickey" ) {
                pubkey = true;
              }
              else if( s == "password" ) {
                pass = true;
              }
              else if( s == "keyboard-interactive" ) {
                keybd = true;
              }
              else {
                slog( "Unknown/unsupported authentication type '%s'", s.c_str() );
              }
            });

            if( pubkey ) {
              if( try_pub_key() ) 
                return;
            }
            if( pass ) {
              if( try_pass() )
                return;
            }
            if( keybd ) {
              if( try_keyboard() )
                return;
            }
            FC_THROW_MSG( "Unable to authenticate" );
        } // authenticate()

        bool try_pass() {
            int ec = libssh2_userauth_password(session, uname.c_str(), upass.c_str() );
            while( ec == LIBSSH2_ERROR_EAGAIN ) {
              wait_on_socket();
              ec = libssh2_userauth_password(session, uname.c_str(), upass.c_str() );
            }

            return !ec;
        }
        bool try_keyboard() {
            int ec = libssh2_userauth_keyboard_interactive(session, uname.c_str(), 
                                                             &client_impl::kbd_callback);
            while( ec == LIBSSH2_ERROR_EAGAIN ) {
              wait_on_socket();
              ec = libssh2_userauth_keyboard_interactive(session, uname.c_str(), 
                                                              &client_impl::kbd_callback);
            }
            return !ec;
        }

        bool try_pub_key() {
            int ec = libssh2_userauth_publickey_fromfile(session,
                                                       uname.c_str(),
                                                       pubkey.c_str(),
                                                       privkey.c_str(),
                                                       passphrase.c_str() );
   
            while( ec == LIBSSH2_ERROR_EAGAIN ) {
              wait_on_socket();
              ec = libssh2_userauth_publickey_fromfile(session,
                                                       uname.c_str(),
                                                       pubkey.c_str(),
                                                       privkey.c_str(),
                                                       passphrase.c_str() );
            }
            return !ec;
        }

        void wait_on_socket() {
          auto dir = libssh2_session_block_directions(session);
          if( !dir ) return;

          fc::promise<boost::system::error_code>::ptr rprom, wprom;
          if( dir & LIBSSH2_SESSION_BLOCK_INBOUND ) {
            rprom = read_prom;
            if(!rprom.get()) {
          //     elog( "   this %2%            NEW READ PROM      %1%           ", read_prom.get(), this );
               read_prom.reset( new fc::promise<boost::system::error_code>("read_prom") );
           //    wlog( " new read prom %1%   this %2%", read_prom.get(), this );
               rprom = read_prom;
               sock->async_read_some( boost::asio::null_buffers(),
                                        [=]( const boost::system::error_code& e, size_t  ) {
                                          this->read_prom->set_value(e);
                                          this->read_prom.reset(0);
                                        } );
            } else {
      //        elog( "already waiting on read %1%", read_prom.get() );
            }
          }
          
          if( dir & LIBSSH2_SESSION_BLOCK_OUTBOUND ) {
            wprom = write_prom;
            if( !write_prom ) {
                write_prom.reset( new fc::promise<boost::system::error_code>("write_prom") );
                wprom = write_prom;
                sock->async_write_some( boost::asio::null_buffers(),
                                         [=]( const boost::system::error_code& e, size_t  ) {
                                            this->write_prom->set_value(e);
                                            this->write_prom.reset(0);
                                         } );
            } else {
        //      elog( "already waiting on write" );
            }
          }


          boost::system::error_code ec;
          if( rprom.get() && wprom.get() ) {
           // elog( "************* Attempt to wait in either direction currently waits for both directions ****** " );
            //wlog( "rprom %1%   wprom %2%", rprom.get(), write_prom.get() );
        //     wlog( "wait on read %1% or write %2% ", rprom.get(), wprom.get() );
            typedef fc::future<boost::system::error_code> fprom;
            fprom fw(wprom);
            fprom fr(rprom);
            int r = fc::wait_any( fw, fr );
            switch( r ) {
              case 0:
                break;
              case 1:
                break;
            }
          } else if( rprom ) {
              if( rprom->wait() ) { 
                FC_THROW( boost::system::system_error(rprom->wait() ) ); 
              }
          } else if( wprom ) {
              if( wprom->wait() ) { FC_THROW( boost::system::system_error(wprom->wait() ) ); }
          }
        }
        void init_sftp() {
          if( !sftp ) {
             sftp = libssh2_sftp_init(session);
             while( !sftp ) {
                char* msg = 0;
                int   ec = libssh2_session_last_error(session,&msg,NULL,0);
                if( ec == LIBSSH2_ERROR_EAGAIN ) {
                  wait_on_socket();
                  sftp = libssh2_sftp_init(session);
                } else {
                  FC_THROW_MSG( "init sftp error %s: %s", ec, msg );
                }
             }
          }
        }


    };
  }

  client::client():my( new detail::client_impl() ){}
  client::~client(){}

  void client::connect( const fc::string& user, const fc::string& host, uint16_t port ) {
       my->hostname = host;
       my->uname    = user;
       my->port     = port;
       my->connect();
  }
  void client::connect( const fc::string& user, const fc::string& pass, 
                        const fc::string& host, uint16_t port  ) {
       my->hostname = host;
       my->uname    = user;
       my->upass    = pass;
       my->port     = port;

       my->connect();
  }


  ssh::process client::exec( const fc::string& cmd, const fc::string& pty_type  ) {
    return ssh::process( *this, cmd, pty_type ); 
  }

  void client::scp_send( const fc::path& local_path, const fc::path& remote_path, 
                        fc::function<bool,size_t,size_t> progress ) {
    /**
     *  Tests have shown that if one scp is 'blocked' by a need to read (presumably to 
     *  ack recv for the trx window), and then a second transfer begins that the first
     *  transfer will never be acked.   Placing this mutex limits the transfer of
     *  one file at a time via SCP which is just as well because there is a fixed
     *  amount of bandwidth.  
     */
    fc::unique_lock<fc::mutex> lock(my->scp_send_mutex);


//    using namespace boost::filesystem;
    if( !fc::exists(local_path) ) {
      FC_THROW_MSG( "Source file '%s' does not exist", local_path.string() );
    }
    if( is_directory( local_path ) ) {
      FC_THROW_MSG( "Source path '%s' is a directory, expected a file.", local_path.string());
    }

    // memory map the file
    file_mapping fmap( local_path.string().c_str(), read_only );
    size_t       fsize = file_size(local_path);

    mapped_region mr( fmap, fc::read_only, 0, fsize );

    LIBSSH2_CHANNEL*                      chan = 0;
    time_t now;
    memset( &now, 0, sizeof(now) );
    // TODO: preserve creation / modification date
    chan = libssh2_scp_send64( my->session, remote_path.string().c_str(), 0700, fsize, now, now );
    while( chan == 0 ) {
      char* msg;
      int ec = libssh2_session_last_error( my->session, &msg, 0, 0 );
      if( ec == LIBSSH2_ERROR_EAGAIN ) {
        my->wait_on_socket();
        chan = libssh2_scp_send64( my->session, local_path.string().c_str(), 0700, fsize, now, now );
      } else {
          FC_THROW_MSG( "scp %s to %s failed %s - %s",local_path.string(), remote_path.string(), ec, msg );
      }
    }
    try {
      uint64_t   wrote = 0;
      char* pos = reinterpret_cast<char*>(mr.get_address());
      while( progress( wrote, fsize ) && wrote < fsize ) {
          int r = libssh2_channel_write( chan, pos, fsize - wrote );
          while( r == LIBSSH2_ERROR_EAGAIN ) {
            my->wait_on_socket();
            r = libssh2_channel_write( chan, pos, fsize - wrote );
          }
          if( r < 0 ) {
             char* msg = 0;
             int ec = libssh2_session_last_error( my->session, &msg, 0, 0 );
             FC_THROW_MSG( "scp failed %s - %s", ec, msg );
          }
          wrote += r;
          pos   += r;
      } 
    } catch ( ... ) {
      // clean up chan
      int ec = libssh2_channel_free(chan );  
      while( ec == LIBSSH2_ERROR_EAGAIN ) {
        my->wait_on_socket();
        ec = libssh2_channel_free( chan );  
      }
      throw;
    }
    int ec = libssh2_channel_free( chan );  
    while( ec == LIBSSH2_ERROR_EAGAIN ) {
      my->wait_on_socket();
      ec = libssh2_channel_free( chan );  
    }
    if( ec < 0 ) {
       char* msg = 0;
       int ec = libssh2_session_last_error( my->session, &msg, 0, 0 );
       FC_THROW_MSG( "scp failed %s - %s", ec, msg );
    }
    
  }


  void client::rm( const fc::path& remote_path ) {
    auto s = stat(remote_path);
    if( s.is_directory() ) {
      FC_THROW_MSG( "Directory exists at path %s", remote_path.string() );
    }
    else if( !s.exists() ) {
      return; // nothing to do
    }

    int rc = libssh2_sftp_unlink(my->sftp, remote_path.string().c_str() );
    while( rc == LIBSSH2_ERROR_EAGAIN ) {
      my->wait_on_socket();
      rc = libssh2_sftp_unlink(my->sftp, remote_path.string().c_str() );
    }
    if( 0 != rc ) {
       rc = libssh2_sftp_last_error(my->sftp);
       FC_THROW_MSG( "rm error %s", rc );
    }
  }

  file_attrib client::stat( const fc::path& remote_path ){
     my->init_sftp();
     LIBSSH2_SFTP_ATTRIBUTES att;
     int ec = libssh2_sftp_stat( my->sftp, remote_path.string().c_str(), &att );
     while( ec == LIBSSH2_ERROR_EAGAIN ) {
        my->wait_on_socket();
        ec = libssh2_sftp_stat( my->sftp, remote_path.string().c_str(), &att );
     }
     if( ec ) {
        return file_attrib();
     }
     file_attrib    ft;
     ft.size        = att.filesize;
     ft.permissions = att.permissions;
     return ft;
  }

  void client::mkdir( const fc::path& remote_dir, int mode ) {
  }

  void client::close() {
  }
 
} }
