#pragma once
#include <fc/ssh/process.hpp>
#include <functional>
#include <fc/filesystem.hpp>

namespace fc { 
  class path;
  namespace ssh {
  namespace detail {
    class client_impl;
    class process_impl;
  };

  enum sftp_file_type {
    named_pipe = 0010000,
    directory  = 0040000,
    regular    = 0100000,
    symlink    = 0120000
  };


  enum sftp_file_mode {
    owner_mask    =    0000700,     /* RWX mask for owner */
    owner_read    =    0000400,     /* R for owner */
    owner_write   =    0000200,     /* W for owner */
    owner_exec    =    0000100,     /* X for owner */
    group_mask    =    0000070,     /* RWX mask for group */
    group_read    =    0000040,     /* R for group */
    group_write   =    0000020,     /* W for group */
    group_exec    =    0000010,     /* X for group */
    other_mask    =    0000007,     /* RWX mask for other */
    other_read    =    0000004,     /* R for other */
    other_write   =    0000002,     /* W for other */
    other_exec    =    0000001      /* X for other */
  };

  struct file_attrib {
    file_attrib();

    uint64_t size;
    uint32_t uid;
    uint32_t gid;
    uint32_t permissions;
    uint32_t atime;
    uint32_t mtime;

    bool     exists();
    bool     is_file();
    bool     is_directory();
  };


  /**
   *  @brief Enables communication over ssh using libssh2.
   *
   *  Because the client creates other resources that depend upon
   *  it, it can only be created as a std::shared_ptr<client> (aka client::ptr) 
   *  via client::create();
   */
  class client {
    public:
      void connect( const fc::string& user, const fc::string& host, uint16_t port = 22);

      /**
       *  Connect via password or keyboard-interactive 
       */
      void connect( const fc::string& user, const fc::string& pass, const fc::string& host, uint16_t port = 22);

      /**
       *  @brief execute command on remote machine
       *  @param pty_type - whether or not to request a PTY when executing this process, this is necessary 
       *          for interactive (non-buffered) IO with the remote process, if left empty no pty will be 
       *          requested
       *
       *  @note Processes launched in this manner will fully buffer stdin and stdout regardless of whether
       *     the process calls flush().  If you need unbuffered (streaming, realtime) access to standard
       *     out then you must launch the process via a shell.
       */
      ssh::process exec( const fc::string& cmd, const fc::string& pty_type = "" );


      /**
       *  @brief upload a file to remote host
       *  @param progress a callback to report / cancel upload.  
       *         The callback takes two parameters, bytes sent and file size.  To continue the 
       *         transfer, the callback should return true.  To cancel the callback should return false.
       */
      void scp_send( const fc::path& local_path, const fc::path& remote_path, 
                              std::function<bool(size_t,size_t)> progress = [](size_t,size_t){return true;} );


      /**
       *  @pre remote_path is not a directory
       *  @post remote file is removed from the remote filesystem
       */
      void rm( const fc::path& remote_path );
      file_attrib stat( const fc::path& remote_path );

      /**
       *  @pre all parent directories already exist.
       *  @pre remote_dir is not exist or is already a directory
       *  @post remote_dir exists.
       */
      void mkdir( const fc::path& remote_dir, int mode = owner_read|owner_write|owner_exec );

      void close();

      client();
      ~client();

    private:
      friend class process;
      friend class detail::process_impl;
      fc::shared_ptr<detail::client_impl>  my;
  };

} } // namespace fc::ssh
