#pragma once

namespace std {
  template<typename> class function;
}

namespace benchmark
{
  struct instance
  {
    inline void start(const std::string& name, bool verbose = true);
    inline uint64_t repeat(const std::string& name, uint32_t count, const std::function<void()>& func, bool verbose = true);
    inline uint64_t once(const std::string& name, const std::function<void()>& func, bool verbose = true);
    inline uint64_t stop();

  private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_point, end_point;
    std::string name;
    bool verbose = true;
  };

  struct scope_counter
  {
    explicit scope_counter(const std::string& name, bool verbose = true);
    ~scope_counter();

    instance state;
  };  

  static instance static_instance;
  static void static_start(const std::string& name, bool verbose = true)
  {
    static_instance.start(name, verbose);
  }
  static uint64_t static_stop()
  {
    return static_instance.stop();
  }
}