#pragma once
#include <fc/string.hpp>

namespace fc {
  struct future_wait_timeout: public std::exception{
     future_wait_timeout( const fc::string& msg = "" ):m_msg(msg){}
     ~future_wait_timeout()throw() {}
     const char*  what()const throw() { return m_msg.c_str(); }
     private:
       fc::string m_msg;
  };
  struct task_canceled: public std::exception{};
  struct thread_quit: public std::exception{};
  struct wait_any_error: public std::exception{};

  struct pke_exception : public std::exception {};
  struct invalid_buffer_length : public  pke_exception {};
  struct invalid_key_length : public pke_exception {};

  struct generic_exception : public std::exception {
     generic_exception( const fc::string& msg = "" ):m_msg(msg){}
     ~generic_exception()throw() {}
     const char*  what()const throw() { return m_msg.c_str(); }
     private:
       fc::string m_msg;
  };


  struct bad_cast: public std::exception{
    const char* what()const throw(){ return "bad cast"; }
  };
  struct range_error: public std::exception{
    const char* what()const throw(){ return "range error"; }
  };
}

