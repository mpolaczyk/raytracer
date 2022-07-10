#include "stdafx.h"

#include "thread_pool.h"

extern void seh_exception_handler(unsigned int u, _EXCEPTION_POINTERS* pExp);

void thread_pool::queue_job(const std::function<void()>& job)
{
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    jobs.push(job);
  }
  mutex_condition.notify_one();
}

void thread_pool::start(uint32_t num_threads)
{
  uint32_t n = num_threads != 0 ? num_threads : std::thread::hardware_concurrency();
  std::cout << "Starting with threads: " << n << std::endl;
  threads.resize(n);
  for (uint32_t i = 0; i < n; i++)
  {
    threads.at(i) = std::thread(&thread_pool::thread_loop, this);
  }
}

void thread_pool::thread_loop()
{
#if USE_FPEXCEPT
  if (!IsDebuggerPresent())
  {
    // Register SEH exception catching when no debugger is present
    _set_se_translator(seh_exception_handler);
  }
  fpexcept::enabled_scope fpe;
#endif

  try
  {
    while (true)
    {
      std::function<void()> job;
      {
        std::unique_lock<std::mutex> lock(queue_mutex);
        mutex_condition.wait(lock,
          [this]
          {
            return !jobs.empty() || should_terminate;
          });
        if (should_terminate)
        {
          return;
        }
        job = jobs.front();
        jobs.pop();
      }
      job();
    }
  }
  catch (const std::exception& e)
  {
    std::cout << "Exception handler:" << std::endl;
    std::cout << e.what() << std::endl;
    __debugbreak();
    system("pause");
  }
}

bool thread_pool::is_busy()
{
  bool pool_empty;
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    pool_empty = jobs.empty();
  }
  return !pool_empty;
}

void thread_pool::stop()
{
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    should_terminate = true;
  }
  mutex_condition.notify_all();
  for (std::thread& active_thread : threads) 
  {
    active_thread.join();
  }
  threads.clear();
}