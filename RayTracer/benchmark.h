#pragma once

#include <chrono>
#include <functional>
#include <iostream>

using namespace std::chrono;

time_point<high_resolution_clock> start_point, end_point;

namespace benchmark
{
  inline void start()
  {
    start_point = high_resolution_clock::now();
  }

  inline int64_t stop(const char* name)
  {
    end_point = high_resolution_clock::now();
    int64_t start = time_point_cast<microseconds>(start_point).time_since_epoch().count();
    int64_t end = time_point_cast<microseconds>(end_point).time_since_epoch().count();
    int64_t time = end - start;
    std::cout << name << ": " << time << "[us]" << std::endl;
    return time;
  }

  inline int64_t repeat(int count, const char* name, std::function<void()> func)
  {
    start();
    for (int i = 0; i < count; i++)
    {
      func();
    }
    return stop(name);
  }

  inline int64_t once(const char* name, std::function<void()> func)
  {
    return repeat(1, name, func);
  }
}