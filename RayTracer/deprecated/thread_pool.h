#pragma once

#include <functional>
#include <mutex>
#include <vector>
#include <queue>

class thread_pool
{
public:
  void start(uint32_t num_threads = 0);
  void queue_job(const std::function<void()>& job);
  void stop();
  bool is_busy();

private:
  void thread_loop();

  bool should_terminate = false;           // Tells threads to stop looking for jobs
  std::mutex queue_mutex;                  // Prevents data races to the job queue
  std::condition_variable mutex_condition; // Allows threads to wait on new jobs or termination 
  std::vector<std::thread> threads;
  std::queue<std::function<void()>> jobs;
};
