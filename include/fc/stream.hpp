#ifndef _FC_STREAM_HPP_
#define _FC_STREAM_HPP_
#include <fc/utility.hpp>
#include <fc/fwd.hpp>

namespace fc {
  class string;
  namespace detail {
    template<typename T>
    struct has_close {
      typedef char (&no_tag)[1];
      typedef char (&yes_tag)[2];

      template<typename C, void (C::*)() > struct has_close_helper{};

      template<typename C >
      static no_tag has_member_helper(...);

      template<typename C>
      static yes_tag has_member_helper( has_close_helper<C,&C::close>* p);
      
      enum closed_value {  value = sizeof(has_member_helper<T>(0)) == sizeof(yes_tag) };
    };

    template<typename C, bool HasClose = has_close<C>::value>
    struct if_close { static void close( C& c ) { c.close(); } };

    template<typename C>
    struct if_close<C,false> { static void close( C& ) { } };



    class abstract_istream {
      public:
      abstract_istream();
      virtual ~abstract_istream();
      size_t readsome( char* buf, size_t len );

      virtual size_t readsome_impl( char* buf, size_t len ) = 0;

      //private:
      // store a boost::iostreams device that will do
      // the actual reading/writing for the stream operators
      //void* _store[51];
      char _store[51*sizeof(void*)];
    };

    template<typename IStream>
    class istream : public abstract_istream {
      public:
      istream( IStream& i ):_in(i){}

      virtual size_t readsome_impl( char* buf, size_t len ) {
        return _in.readsome(buf,len);
      }

      private:
        IStream& _in;
    };

    class abstract_ostream {
      public:
        abstract_ostream();
        virtual ~abstract_ostream();
        size_t write( const char* buf, size_t len );
        void   close();
        void   flush();

        virtual void   close_impl() = 0;
        virtual void   flush_impl() = 0;
        virtual size_t write_impl( const char* buf, size_t len ) = 0; 
//      private:
        // store a boost::iostreams device that will do
        // the actual reading/writing for the stream operators
        void* _store[50];
    };

    template<typename OStream>
    class ostream : public abstract_ostream {
      public:
      ostream( OStream& o ):_out(o){}

      virtual size_t write_impl( const char* buf, size_t len ) {
        _out.write(buf,len);
        return len;
      }
      virtual void close_impl() { if_close<OStream>::close(_out); }
      virtual void flush_impl() { _out.flush(); }
      
      private:
        OStream& _out;
    };

  }

  class istream {
    public:
      template<typename IStream>
      istream( IStream& is ) {
        static_assert( sizeof(detail::istream<IStream>(is)) <= sizeof(_store), "Failed to reserve enough space");
        new ((void*)&_store[0]) detail::istream<IStream>(is);
      }
      ~istream();

      size_t readsome( char* buf, size_t len );

      friend istream& operator>>( istream&, int64_t&    );
      friend istream& operator>>( istream&, uint64_t&   );
      friend istream& operator>>( istream&, int32_t&    );
      friend istream& operator>>( istream&, uint32_t&   );
      friend istream& operator>>( istream&, int16_t&    );
      friend istream& operator>>( istream&, uint16_t&   );
      friend istream& operator>>( istream&, int8_t&     );
      friend istream& operator>>( istream&, uint8_t&    );
      friend istream& operator>>( istream&, float&      );
      friend istream& operator>>( istream&, double&     );
      friend istream& operator>>( istream&, bool&       );
      friend istream& operator>>( istream&, char&       );
      friend istream& operator>>( istream&, fc::string& );

    private:
      istream( const istream& );
      istream& operator=(const istream& );
      void*  _store[54];
  };

  class ostream {
    public:
      template<typename OStream>
      ostream( OStream& os ) {
        static_assert( sizeof(detail::ostream<OStream>(os)) <= sizeof(_store), "Failed to reserve enough space");
        new ((void*)&_store[0]) detail::ostream<OStream>(os);
      }

      ~ostream();

      size_t write( const char* buf, size_t len );
      void   close();
      void   flush();

      friend ostream& operator<<( ostream&, int64_t           );
      friend ostream& operator<<( ostream&, uint64_t          );
      friend ostream& operator<<( ostream&, int32_t           );
      friend ostream& operator<<( ostream&, uint32_t          );
      friend ostream& operator<<( ostream&, int16_t           );
      friend ostream& operator<<( ostream&, uint16_t          );
      friend ostream& operator<<( ostream&, int8_t            );
      friend ostream& operator<<( ostream&, uint8_t           );
      friend ostream& operator<<( ostream&, float             );
      friend ostream& operator<<( ostream&, double            );
      friend ostream& operator<<( ostream&, bool              );
      friend ostream& operator<<( ostream&, char              );
      friend ostream& operator<<( ostream&, const char*       );
      friend ostream& operator<<( ostream&, const fc::string& );

    private:
      ostream( const ostream& o );
      ostream& operator=(const ostream& o);
      void*  _store[54];
  };

  bool getline( fc::istream&, fc::string& );

  class stringstream {
    public:
      stringstream();
      stringstream( fc::string& s);
      ~stringstream();

      fc::string str();

      friend stringstream& operator>>( stringstream&, int64_t&    );
      friend stringstream& operator>>( stringstream&, uint64_t&   );
      friend stringstream& operator>>( stringstream&, int32_t&    );
      friend stringstream& operator>>( stringstream&, uint32_t&   );
      friend stringstream& operator>>( stringstream&, int16_t&    );
      friend stringstream& operator>>( stringstream&, uint16_t&   );
      friend stringstream& operator>>( stringstream&, int8_t&     );
      friend stringstream& operator>>( stringstream&, uint8_t&    );
      friend stringstream& operator>>( stringstream&, float&      );
      friend stringstream& operator>>( stringstream&, double&     );
      friend stringstream& operator>>( stringstream&, bool&       );
      friend stringstream& operator>>( stringstream&, char&       );
      friend stringstream& operator>>( stringstream&, fc::string& );

      friend stringstream& operator<<( stringstream&, const int64_t&    );
      friend stringstream& operator<<( stringstream&, const uint64_t&   );
      friend stringstream& operator<<( stringstream&, const int32_t&    );
      friend stringstream& operator<<( stringstream&, const uint32_t&   );
      friend stringstream& operator<<( stringstream&, const int16_t&    );
      friend stringstream& operator<<( stringstream&, const uint16_t&   );
      friend stringstream& operator<<( stringstream&, const int8_t&     );
      friend stringstream& operator<<( stringstream&, const uint8_t&    );
      friend stringstream& operator<<( stringstream&, const float&      );
      friend stringstream& operator<<( stringstream&, const double&     );
      friend stringstream& operator<<( stringstream&, const bool&       );
      friend stringstream& operator<<( stringstream&, const char&       );
      friend stringstream& operator<<( stringstream&, const fc::string& );
    private:
      class impl;
      fwd<impl,368> my;
  };

  class ofstream {
    public:
      enum mode { out, binary };
      ofstream();
      ofstream( const fc::string& file, int m );
      ~ofstream();

      void open( const fc::string& file, int m );
      ofstream& write( const char* buf, size_t len );
      void   put( char c );
      void   close();
      void   flush();

    private:
      class impl;
      fwd<impl,896> my;
  };
  class ifstream {
    public:
      enum mode { in, binary };
      ifstream();
      ifstream( const fc::string& file, int m );
      ~ifstream();

      void open( const fc::string& file, int m );
      ifstream& read( char* buf, size_t len );
      void   close();
      void   flush();

    private:
      class impl;
      fwd<impl,904> my;
  };

  extern ostream cout;
  extern ostream cerr;
  extern istream cin;
}

#endif // _FC_STREAM_HPP_
