#include <fc/filesystem.hpp>
#include <fc/value_cast.hpp>
#include <boost/filesystem.hpp>
#include <fc/ssh/client.hpp>
#include <fc/ssh/process.hpp>
#include <fc/exception.hpp>
#include <fc/iostream.hpp>
#include <fc/log.hpp>
#include <fc/thread.hpp>
#include <fc/vector.hpp>
#include <fc/interprocess/file_mapping.hpp>
#include <fc/unique_lock.hpp>
#include <fc/mutex.hpp>
#include <fc/error_report.hpp>
#include <libssh2.h>
#include <libssh2_sftp.h>
#include <memory>
#include <fc/asio.hpp>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

namespace fc { namespace ssh {
  
  namespace detail {
    static int ssh_init = libssh2_init(0);

    class process_impl;
    class process_istream : public fc::istream {
       public:
          process_istream( process_impl& p, int c )
          :proc(p),chan(c){}

          virtual size_t readsome( char* buf, size_t len );
          virtual istream& read( char* buf, size_t len );

          virtual bool eof()const;

          process_impl& proc;
          int           chan;
    };

    class process_ostream : public fc::ostream {
      public:
          process_ostream( process_impl& p )
          :proc(p){}

          virtual ostream& write( const char* buf, size_t len );
          virtual void   close();
          virtual void   flush();

          process_impl& proc;
    };

    class process_impl : public fc::retainable {
      public:
        process_impl( const client& c, const fc::string& cmd, const fc::string& pty_type );
        fc::string            command;
        fc::promise<int>::ptr result;
        LIBSSH2_CHANNEL*      chan;

        int read_some( char* data, size_t len, int stream_id );
        int write_some( const char* data, size_t len, int stream_id );
        void flush();
        void send_eof();

        client  sshc;
        process_istream std_err;
        process_istream std_out;
        process_ostream std_in;

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

        LIBSSH2_CHANNEL*   open_channel( const fc::string& pty_type );
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
            if( libssh2_init(0) < 0  ) { FC_THROW_REPORT( "Unable to init libssh2" ); }
           
            auto eps = fc::asio::tcp::resolve( hostname, fc::lexical_cast<fc::string>(port) );
            if( eps.size() == 0 ) {
               FC_THROW_REPORT( "Unable to resolve host '${host}'", fc::value().set("host",hostname) );
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
              FC_THROW_REPORT( "SSH Handshake error: ${code} - ${message}", 
                               fc::value().set("code",ec).set("message", msg) );
            }
            /*const char* fingerprint = */libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA1);
             
            // try to authenticate, throw on error.
            authenticate();
          } catch ( error_report& er ) {
             close();
             throw FC_REPORT_PUSH( er, "Unable to connect to ssh server" );;
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
           try {
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
                    FC_THROW_REPORT( "Error getting authorization list: ${code} - ${message}", 
                                     fc::value().set("code",ec).set("message",msg));
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
           } catch ( error_report er ) {
              throw FC_REPORT_PUSH( er, "Unable to authenticate ssh connection" );
           }
           FC_THROW_REPORT( "Unable to authenticate ssh connection" );
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
                FC_THROW_REPORT( "Socket Error ${message}", 
                                 fc::value().set( "message", boost::system::system_error(rprom->wait() ).what() ) ); 
              }
          } else if( wprom ) {
              if( wprom->wait() ) { 
                FC_THROW_REPORT( "Socket Error ${message}", 
                                 fc::value().set( "message", boost::system::system_error(wprom->wait() ).what() ) ); 
              }
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
                  FC_THROW_REPORT( "init sftp error ${code} - ${message}", fc::value().set("code",ec).set("message",msg) );
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
                        std::function<bool(size_t,size_t)> progress ) {
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
      FC_THROW_REPORT( "Source file '${file}' does not exist", fc::value().set("file",local_path) ) ;
    }
    if( is_directory( local_path ) ) {
      FC_THROW_REPORT( "Source file '${file}' is a directory, expected a file", fc::value().set("file",local_path) ) ;
    }

    // memory map the file
    file_mapping fmap( local_path.string().c_str(), read_only );
    size_t       fsize = file_size(local_path);

    mapped_region mr( fmap, fc::read_only, 0, fsize );

    LIBSSH2_CHANNEL*                      chan = 0;
    time_t now;
    memset( &now, 0, sizeof(now) );
    // TODO: preserve creation / modification date
    chan = libssh2_scp_send64( my->session, remote_path.generic_string().c_str(), 0700, fsize, now, now );
    while( chan == 0 ) {
      char* msg;
      int ec = libssh2_session_last_error( my->session, &msg, 0, 0 );
      if( ec == LIBSSH2_ERROR_EAGAIN ) {
        my->wait_on_socket();
        chan = libssh2_scp_send64( my->session, local_path.generic_string().c_str(), 0700, fsize, now, now );
      } else {
         FC_THROW_REPORT( "scp ${local_file} to ${remote_file} failed ${code} - ${message}", 
          fc::value().set("local_file", local_path).set("remote_file",remote_path).set("code",ec).set("message",msg) );
      }
    }
    try {
      size_t   wrote = 0;
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
             FC_THROW_REPORT( "scp failed ${code} - ${message}", 
                              fc::value().set("code",ec).set("message",msg) );
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
       FC_THROW_REPORT( "scp failed ${code} - ${message}", 
                        fc::value().set("code",ec).set("message",msg) );
    }
    
  }


  void client::rm( const fc::path& remote_path ) {
    try {
       auto s = stat(remote_path);
       if( s.is_directory() ) {
         FC_THROW_REPORT( "sftp cannot remove directory ${path}", fc::value().set("path",remote_path) );
       }
       else if( !s.exists() ) {
         return; // nothing to do
       }

       int rc = libssh2_sftp_unlink(my->sftp, remote_path.generic_string().c_str() );
       while( rc == LIBSSH2_ERROR_EAGAIN ) {
         my->wait_on_socket();
         rc = libssh2_sftp_unlink(my->sftp, remote_path.generic_string().c_str() );
       }
       if( 0 != rc ) {
          rc = libssh2_sftp_last_error(my->sftp);
          FC_THROW_REPORT( "sftp rm failed ${code}", fc::value().set("code",rc) );
       }
     } catch ( error_report& er ) {
         throw FC_REPORT_PUSH( er, "sftp remove '${remote_path}' failed", fc::value().set("remote_path",remote_path) );
     }
  }

  file_attrib client::stat( const fc::path& remote_path ){
     my->init_sftp();
     LIBSSH2_SFTP_ATTRIBUTES att;
     int ec = libssh2_sftp_stat( my->sftp, remote_path.generic_string().c_str(), &att );
     while( ec == LIBSSH2_ERROR_EAGAIN ) {
        my->wait_on_socket();
        ec = libssh2_sftp_stat( my->sftp, remote_path.generic_string().c_str(), &att );
     }
     if( ec ) {
        return file_attrib();
     }
     file_attrib    ft;
     ft.size        = att.filesize;
     ft.permissions = att.permissions;
     return ft;
  }
  void client::create_directories( const fc::path& rdir, int mode ) {
     boost::filesystem::path dir = rdir;
     boost::filesystem::path p;
     auto pitr = dir.begin();
     while( pitr != dir.end() ) {
       p /= *pitr;
       if( !stat( p ).exists() ) {
         mkdir(p,mode);
       } 
       ++pitr;
     }
  }

  void client::mkdir( const fc::path& rdir, int mode ) {
     try {
       auto s = stat(rdir);
       if( s.is_directory() ) return;
       else if( s.exists() ) {
         FC_THROW_REPORT( "File already exists at path ${path}", fc::value().set("path",rdir) );
       }

       int rc = libssh2_sftp_mkdir(my->sftp, rdir.generic_string().c_str(), mode );
       while( rc == LIBSSH2_ERROR_EAGAIN ) {
         my->wait_on_socket();
         rc = libssh2_sftp_mkdir(my->sftp, rdir.generic_string().c_str(), mode );
       }
       if( 0 != rc ) {
          rc = libssh2_sftp_last_error(my->sftp);
          FC_THROW_REPORT( "sftp mkdir error", fc::value().set("code",rc) );
       }
     } catch ( error_report& er ) {
          throw FC_REPORT_PUSH( er, "sftp failed to create directory '${directory}'", fc::value().set( "directory", rdir ) );
     }
  }

  void client::close() {
    if( my->session ) {
       if( my->sftp ) {
         int ec = libssh2_sftp_shutdown(my->sftp);
         try {
             while( ec == LIBSSH2_ERROR_EAGAIN ) {
                my->wait_on_socket();
                ec = libssh2_sftp_shutdown(my->sftp);
             } 
         }catch(...){
          elog( "... caught error closing sftp session???" );
         }
         my->sftp = 0;
       }
       try {
         int ec = libssh2_session_disconnect(my->session, "exit cleanly" );
         while( ec == LIBSSH2_ERROR_EAGAIN ) {
            my->wait_on_socket();
            ec = libssh2_session_disconnect(my->session, "exit cleanly" );
         }
         ec = libssh2_session_free(my->session);
         while( ec == LIBSSH2_ERROR_EAGAIN ) {
            my->wait_on_socket();
            ec = libssh2_session_free(my->session );
         }
         my->session = 0;
       } catch ( ... ){
          elog( "... caught error freeing session???" );
          my->session = 0;
       }
       try {
         if( my->sock ) {
           slog( "closing socket" );
           my->sock->close();
         }
       } catch ( ... ){
          elog( "... caught error closing socket???" );
       }
       my->sock.reset(0);
       try {
        if( my->read_prom ) my->read_prom->wait();
       } catch ( ... ){
        wlog( "caught error waiting on read prom" );
       }
       try {
        if( my->write_prom ) my->write_prom->wait();
       } catch ( ... ){
        wlog( "caught error waiting on write prom" );
       }
    }
  }

  file_attrib::file_attrib()
  :size(0),uid(0),gid(0),permissions(0),atime(0),mtime(0)
  { }

  bool file_attrib::is_directory() {
    return  LIBSSH2_SFTP_S_ISDIR(permissions);
  }
  bool file_attrib::is_file() {
    return LIBSSH2_SFTP_S_ISREG(permissions);
  }
  bool file_attrib::exists() {
    return 0 != permissions;
  }
  



  process::process()
  {}
  process::~process()
  {}
  bool process::valid()const {
      return !!my;
  }

  /**
   *  Blocks until the result code of the process has been returned.
   */
  int process::result() {
    return 0;
  }
  /**
   *  @brief returns a stream that writes to the procss' stdin
   */
  fc::ostream& process::in_stream()const {
    return my->std_in;
  }
  /**
   *  @brief returns a stream that reads from the process' stdout
   */
  fc::istream& process::out_stream()const {
    return my->std_out;
  }
  /**
   *  @brief returns a stream that reads from the process' stderr
   */
  fc::istream& process::err_stream()const {
    return my->std_err;
  }

  process::process( const process& p ) 
  :my(p.my){ }
  process::process( process&& p )
  :my(fc::move(p.my)){ }

  process::process( client& c, const fc::string& cmd, const fc::string& pty_type)
  :my( new detail::process_impl( c, cmd, pty_type ) )
  {
  }


  void detail::process_impl::flush() {
      if( !chan ) return;
      int ec = libssh2_channel_flush_ex( chan, LIBSSH2_CHANNEL_FLUSH_EXTENDED_DATA);
      while( ec == LIBSSH2_ERROR_EAGAIN ) {
        sshc.my->wait_on_socket();
        ec = libssh2_channel_flush_ex( chan, LIBSSH2_CHANNEL_FLUSH_EXTENDED_DATA );
      }
      if( ec < 0 ) {
        FC_THROW_REPORT( "ssh flush failed", fc::value().set( "channel_error", ec)  );
      }
  }
  int detail::process_impl::read_some( char* data, size_t len, int stream_id ){
       if( !sshc.my->session ) { FC_THROW_REPORT( "Session closed" ); }
       
       int rc;
       char* buf = data;
       size_t buflen = len;
       do {
           rc = libssh2_channel_read_ex( chan, stream_id, buf, buflen );
           if( rc > 0 ) {
              buf += rc;
              buflen -= rc;
              return buf-data;
           } else if( rc == 0 ) {
              if( libssh2_channel_eof( chan ) )  {
                return -1; // eof
              }
              sshc.my->wait_on_socket();
           } else {
             if( rc == LIBSSH2_ERROR_EAGAIN ) {
               if( 0 < (buf-data) ) {
                 return buf-data;
               }
               else  {
                 sshc.my->wait_on_socket();
                 rc = 0;
                 continue;
               }
             } else {
               char* msg;
               if( !sshc.my || !sshc.my->session ) { FC_THROW_REPORT( "Session closed" ); }
               rc   = libssh2_session_last_error( sshc.my->session, &msg, 0, 0 );
               FC_THROW_REPORT( "read failed: ${message}", fc::value().set("message", msg).set("code", rc) ); 
             }
           }
       } while( rc >= 0 && buflen);
       return buf-data;
  }
  int detail::process_impl::write_some( const char* data, size_t len, int stream_id ) {
     if( !sshc.my->session ) { FC_THROW_REPORT( "Session closed" ); }

     int rc;
     const char* buf = data;
     size_t buflen = len;
     do {
         rc = libssh2_channel_write_ex( chan, stream_id, buf, buflen );
         if( rc > 0 ) {
            buf += rc;
            buflen -= rc;
            return buf-data;
         } else if( rc == 0 ) {
            if( libssh2_channel_eof( chan ) )  {
              FC_THROW_REPORT( "EOF" );
              //return -1; // eof
            }
         } else {
  
           if( rc == LIBSSH2_ERROR_EAGAIN ) {
             if( 0 < (buf-data) ) {
               return buf-data;
             }
             else  {
               sshc.my->wait_on_socket();
               rc = 0;
               continue;
             }
           } else {
             char* msg;
             rc   = libssh2_session_last_error( sshc.my->session, &msg, 0, 0 );
             FC_THROW_REPORT( "write failed: ${message}", fc::value().set("message",msg).set("code",rc) );
             return buf-data;
           }
         }
     } while( rc >= 0 && buflen);
     return buf-data;
  }
  void detail::process_impl::send_eof() {
    if( sshc.my->session ) {
      int ec = libssh2_channel_send_eof( chan );
      while( ec == LIBSSH2_ERROR_EAGAIN ) {
        sshc.my->wait_on_socket();
        ec = libssh2_channel_send_eof( chan );
      }
      if( ec ) {
        char* msg = 0;
        ec = libssh2_session_last_error( sshc.my->session, &msg, 0, 0 );
        FC_THROW_REPORT( "send eof failed: ${message}", fc::value().set("message",msg).set("code",ec) );
      }
    }
  }

   size_t detail::process_istream::readsome( char* buf, size_t len ) { 
      return proc.read_some( buf, len, chan );
   }
   istream& detail::process_istream::read( char* buf, size_t len )  { 
      size_t r = 0; 
      do { 
        r += proc.read_some( buf + r, len - r, chan );
      } while( r < len );

      return *this; 
   }

   bool detail::process_istream::eof()const { 
      return 0 != libssh2_channel_eof( proc.chan );
   }

   ostream& detail::process_ostream::write( const char* buf, size_t len ) { 
    size_t wrote = 0;
    do {
      wrote += proc.write_some( buf+wrote, len-wrote, 0 );
    } while( wrote < len ); 
    return *this; 
   }
   void   detail::process_ostream::close(){
      proc.send_eof();
   }
   void   detail::process_ostream::flush(){
      proc.flush();
   }
   detail::process_impl::process_impl( const client& c, const fc::string& cmd, const fc::string& pty_type )
   :sshc(c),std_err(*this,SSH_EXTENDED_DATA_STDERR),std_out(*this,0),std_in(*this)
   { 
        chan = c.my->open_channel(pty_type); 

        /*
        unsigned int rw_size = 0;
        int ec = libssh2_channel_receive_window_adjust2(chan, 1024*64, 0, &rw_size );
        while( ec == LIBSSH2_ERROR_EAGAIN ) {
          sshc->my->wait_on_socket();
          ec = libssh2_channel_receive_window_adjust2(chan, 1024*64, 0, &rw_size );
        }
        elog( "rwindow size %1%", rw_size );
        */


        int ec = libssh2_channel_handle_extended_data2(chan, LIBSSH2_CHANNEL_EXTENDED_DATA_NORMAL );
        while( ec == LIBSSH2_ERROR_EAGAIN ) {
          sshc.my->wait_on_socket();
          ec = libssh2_channel_handle_extended_data2(chan, LIBSSH2_CHANNEL_EXTENDED_DATA_NORMAL );
        }

        if( cmd.size() == 0 ) {
            ec = libssh2_channel_shell(chan );
            while( ec == LIBSSH2_ERROR_EAGAIN ) {
              sshc.my->wait_on_socket();
              ec = libssh2_channel_shell(chan);
            }
        } else {
            ec = libssh2_channel_exec( chan, cmd.c_str() );
            while( ec == LIBSSH2_ERROR_EAGAIN ) {
              sshc.my->wait_on_socket();
              ec = libssh2_channel_exec( chan, cmd.c_str() );
            }
        }
        if( ec ) {
           char* msg = 0;
           ec   = libssh2_session_last_error( sshc.my->session, &msg, 0, 0 );
           FC_THROW_REPORT( "exec failed: ${message}", fc::value().set("message",msg).set("code",ec) );
        }
   }
   LIBSSH2_CHANNEL*   detail::client_impl::open_channel( const fc::string& pty_type ) {
        LIBSSH2_CHANNEL*                      chan = 0;
        chan = libssh2_channel_open_session(session);
        if( !chan ) {
           char* msg;
           int ec = libssh2_session_last_error( session, &msg, 0, 0 );
           while( !chan && ec == LIBSSH2_ERROR_EAGAIN ) {
              wait_on_socket();
              chan = libssh2_channel_open_session(session);
              ec   = libssh2_session_last_error( session, &msg, 0, 0 );
           }
           if( !chan ) {
              FC_THROW_REPORT( "libssh2_channel_open_session failed: ${message}", fc::value().set("message",msg).set("code",ec) );
           }
        }

        if( pty_type.size() ) {
            int ec = libssh2_channel_request_pty(chan,pty_type.c_str());
            while( ec == LIBSSH2_ERROR_EAGAIN ) {
               wait_on_socket();
               ec = libssh2_channel_request_pty(chan,pty_type.c_str());
            }
            if( 0 != ec ) {
               char* msg;
               ec = libssh2_session_last_error( session, &msg, 0, 0 );
               FC_THROW_REPORT( "libssh2_channel_req_pty failed: ${message}", fc::value().set("message",msg).set("code",ec) );
            }
        }
        return chan;
    }

   process& process::operator=( const process& p ) {
        my = p.my;
      return *this;
   }
} }
