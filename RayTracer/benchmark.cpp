
#include <iostream>

#include "benchmark.h"

namespace benchmark
{
  void instance::start(const char* in_name)
  {
    name = in_name;
    start_point = high_resolution_clock::now();
  }

  uint64_t instance::repeat(const char* name, uint32_t count, std::function<void()>& func)
  {
    start(name);
    for (uint32_t i = 0; i < count; i++)
    {
      func();
    }
    return stop();
  }

  uint64_t instance::once(const char* name, std::function<void()>& func)
  {
    return repeat(name, 1, func);
  }

  uint64_t instance::stop()
  {
    end_point = high_resolution_clock::now();
    uint64_t start = time_point_cast<microseconds>(start_point).time_since_epoch().count();
    uint64_t end = time_point_cast<microseconds>(end_point).time_since_epoch().count();
    uint64_t time = end - start;
    std::cout << name << ": " << time << "[us] = " << time / 1000 << "[ms] = " << time / 1000000 << "[s]" << std::endl;
    return time;
  }
  
  scope_counter::scope_counter(const char* name)
  {
    state.start(name);
  }

  scope_counter::~scope_counter()
  {
    state.stop();
  }
}
