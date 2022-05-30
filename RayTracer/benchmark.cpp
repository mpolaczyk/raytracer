#include "stdafx.h"

#include "benchmark.h"

namespace benchmark
{
  void instance::start(const char* in_name, bool in_verbose)
  {
    name = in_name;
    verbose = in_verbose;
    start_point = high_resolution_clock::now();
#ifdef USE_PIX
    PIXBeginEvent(PIX_COLOR(155, 1127, 0), in_name);
#endif
  }

  uint64_t instance::repeat(const char* name, uint32_t count, std::function<void()>& func, bool verbose)
  {
    uint64_t sum = 0;
    for (uint32_t i = 0; i < count; i++)
    {
      start(name, verbose);
      func();
      sum += stop();
    }
    return sum;
  }

  uint64_t instance::once(const char* name, std::function<void()>& func, bool verbose)
  {
    return repeat(name, 1, func);
  }

  uint64_t instance::stop()
  {
    end_point = high_resolution_clock::now();
    uint64_t start = time_point_cast<microseconds>(start_point).time_since_epoch().count();
    uint64_t end = time_point_cast<microseconds>(end_point).time_since_epoch().count();
    uint64_t time = end - start;
    if (verbose)
    {
      std::cout << name << ": " << time << "[us] = " << time / 1000 << "[ms] = " << time / 1000000 << "[s]" << std::endl;
    }
#ifdef USE_PIX
    PIXEndEvent();
#endif
    return time;
  }
  
  scope_counter::scope_counter(const char* name, bool verbose)
  {
    state.start(name, verbose);
  }

  scope_counter::~scope_counter()
  {
    state.stop();
  }
}
