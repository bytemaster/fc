

class istream {
  public:
    template<typename T>
    istream( T& s ) {
    
    }

    istream& read( char* buf, uint64_t s );
    int64_t  readsome( char* buf, uint64_t s );
    bool     eof()const;
 
  private:
    struct vtable {
      void*    (*read)(void*, char* buf, uint64_t s );
      int64_t  (*readsome)(void*, char* buf, uint64_t s );
      bool     (*eof)(void* )
    };

    vtable& _vtable;
    void*   _stream;
};




