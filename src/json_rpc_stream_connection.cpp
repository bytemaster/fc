
      /** note the life of i and o must be longer than rpc_connection's life */
      rpc_connection( istream& i, ostream& o );

      /** note the life of i and o must be longer than rpc_connection's life */
      void init( istream& i, ostream& o );

      istream* _in;
      ostream* _out;

      fc::future<void> _read_loop_complete;
      void read_loop() {
        fc::string line;
        fc::getline( *_in, line );
        while( !_in->eof() ) {
            try {
                fc::value v= fc::json::from_string( line );

            } catch (...) {
              wlog( "%s", fc::except_str().c_str() );
            }
            fc::getline( *_in, line );
        }
        slog( "Exit Read Loop, canceling waiting tasks!" );

        auto cur = _pr_head;
        while( cur ) {
          cur->handle_error( "Connection Closed" );
          cur = cur->next;
        }

        _pr_head.reset();
        _pr_tail.reset();
      }

  rpc_connection::rpc_connection( istream& i, ostream& o )
  :my( new impl() )
  {
    init( i, o );
  }
  void rpc_connection::init( istream& i, ostream& o ) {
    my->_in  = &i;
    my->_out = &o;
    my->_read_loop_complete = fc::async( [=](){ my->read_loop(); } );
  }
