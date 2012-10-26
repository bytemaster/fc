#pragma once
#include <fc/iostream.hpp>
#include <fc/shared_ptr.hpp>

namespace fc {
  /**
   *  Used to wrap references to other streams
   */
  class ostream_wrapper : public ostream {
    public:
      template<typename Stream>
      ostream_wrapper( Stream& s )
      :my( new impl<Stream>(s) ){}

      virtual ~ostream_wrapper(){};

      virtual ostream& write( const char* buf, size_t len ) {
        my->write(buf,len);
        return *this;
      }
      virtual void   close() {
        my->close();
      }
      virtual void   flush() {
        my->flush();
      }

    protected:
      virtual ostream& write( const fc::string& s )  {
        return write( s.c_str(), s.size() );
      }

      struct impl_base : public fc::retainable  {
          virtual void write( const char* buf, size_t len ) = 0;
          virtual void close() = 0;
          virtual void flush() = 0;
      };

      template<typename T>
      struct impl : public impl_base {
          impl(T& i):st(i){}

          virtual void write( const char* buf, size_t len ) {
            st.write(buf,len);
          }
          virtual void   close() { st.close(); }
          virtual void   flush() { st.flush(); }
          T& st;
      };
      
      fc::shared_ptr<impl_base> my;
  };
  /**
   *  Used to wrap references to other streams
   */
  class istream_wrapper : public istream {
    public:
      template<typename Stream>
      istream_wrapper( Stream& s )
      :my( new impl<Stream>(s) ){}

      virtual ~istream_wrapper(){};

      virtual size_t readsome( char* buf, size_t len ) { return my->readsome(buf,len); }
      virtual istream& read( char* buf, size_t len ) {
        my->read(buf,len);
        return *this;
      }
      virtual void   close() { }
      virtual bool   eof()const{ return my->eof(); }

      virtual istream& read( int64_t&    ) { return *this; }
      virtual istream& read( uint64_t&   ) { return *this; }
      virtual istream& read( int32_t&    ) { return *this; }
      virtual istream& read( uint32_t&   ) { return *this; }
      virtual istream& read( int16_t&    ) { return *this; }
      virtual istream& read( uint16_t&   ) { return *this; }
      virtual istream& read( int8_t&     ) { return *this; }
      virtual istream& read( uint8_t&    ) { return *this; }
      virtual istream& read( float&      ) { return *this; }
      virtual istream& read( double&     ) { return *this; }
      virtual istream& read( bool&       ) { return *this; }
      virtual istream& read( char&       ) { return *this; }
      virtual istream& read( fc::string& ) { return *this; }

    protected:
      struct impl_base : public fc::retainable  {
          virtual void read( char* buf, size_t len ) = 0;
          virtual size_t readsome( char* buf, size_t len ) = 0;
          virtual bool eof()const;
      };

      template<typename T>
      struct impl : public impl_base {
          impl(T& i):st(i){}

          virtual size_t readsome( char* buf, size_t len ) { return st.readsome(buf,len); }
          virtual void read( char* buf, size_t len ) {
            st.read(buf,len);
          }
          virtual bool eof()const { return st.eof(); }
          T& st;
      };
      
      fc::shared_ptr<impl_base> my;
  };

} // namespace fc

