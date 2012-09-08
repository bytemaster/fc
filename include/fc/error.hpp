#ifndef _FC_ERROR_HPP_
#define _FC_ERROR_HPP_

namespace fc {
  struct future_wait_timeout: public std::exception{};
  struct task_canceled: public std::exception{};
  struct thread_quit: public std::exception{};
  struct wait_any_error: public std::exception{};
  struct bad_cast: public std::exception{
    const char* what()const throw(){ return "bad cast"; }
  };
  struct range_error: public std::exception{
    const char* what()const throw(){ return "range error"; }
  };
}

#endif // _FC_ERROR_HPP_
