#define BOOST_TEST_MODULE fc_task_cancel_tests
#include <boost/test/unit_test.hpp>

#include <fc/thread/thread.hpp>
#include <fc/log/logger.hpp>
#include <fc/exception/exception.hpp>

BOOST_AUTO_TEST_CASE( cancel_an_active_task )
{
  enum task_result{sleep_completed, sleep_aborted};
  fc::future<task_result> task = fc::async([]() {
    BOOST_TEST_MESSAGE("Starting async task");
    try
    {
      fc::usleep(fc::seconds(5));
      return sleep_completed;
    }
    catch (const fc::exception&)
    {
      return sleep_aborted;
    }
  }, "test_task");

  fc::time_point start_time = fc::time_point::now();

  // wait a bit for the task to start running
  fc::usleep(fc::milliseconds(100));

  BOOST_TEST_MESSAGE("Canceling task");
  task.cancel();
  try
  {
    task_result result = task.wait();
    BOOST_CHECK_MESSAGE(result != sleep_completed, "sleep should have been canceled");
  }
  catch (fc::exception& e)
  {
    BOOST_TEST_MESSAGE("Caught exception from canceled task: " << e.what());
    BOOST_CHECK_MESSAGE(fc::time_point::now() - start_time < fc::seconds(4), "Task was not canceled quickly");
  }
}


BOOST_AUTO_TEST_CASE( cleanup_cancelled_task )
{
  std::shared_ptr<std::string> some_string(std::make_shared<std::string>("some string"));
  fc::future<void> task = fc::async([some_string]() {
    BOOST_TEST_MESSAGE("Starting async task, bound string is " << *some_string);
    try
    {
      fc::usleep(fc::seconds(5));
      BOOST_TEST_MESSAGE("Finsihed usleep in async task, leaving the task's functor");
    }
    catch (...)
    {
      BOOST_TEST_MESSAGE("Caught exception in async task, leaving the task's functor");
    }
  }, "test_task");
  std::weak_ptr<std::string> weak_string_ptr(some_string);
  some_string.reset();
  BOOST_CHECK_MESSAGE(!weak_string_ptr.expired(), "Weak pointer should still be valid because async task should be holding the strong pointer");
  fc::usleep(fc::milliseconds(100));
  BOOST_TEST_MESSAGE("Canceling task");
  task.cancel();
  try
  {
    task.wait();
  }
  catch (fc::exception& e)
  {
    BOOST_TEST_MESSAGE("Caught exception from canceled task: " << e.what());
  }
  BOOST_CHECK_MESSAGE(weak_string_ptr.expired(), "Weak pointer should now be invalid because async task should be done with it");
  task = fc::future<void>();
  BOOST_CHECK_MESSAGE(weak_string_ptr.expired(), "Weak pointer should now be invalid because async task should have been destroyed");
}

int task_execute_count = 0;
fc::future<void> simple_task_done;
void simple_task()
{
  task_execute_count++;
  simple_task_done = fc::schedule([](){ simple_task(); }, 
                                  fc::time_point::now() + fc::seconds(3),
                                  "simple_task");
}

BOOST_AUTO_TEST_CASE( cancel_scheduled_task )
{
  bool task_executed = false;
  try 
  {
    simple_task();
    simple_task();
    fc::usleep(fc::seconds(4));
    simple_task_done.cancel();
    simple_task_done.wait();
  } 
  catch ( const fc::exception& e )
  {
    wlog( "${e}", ("e",e.to_detail_string() ) );
  }
  BOOST_CHECK_EQUAL(task_execute_count, 2);
}