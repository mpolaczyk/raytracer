#pragma once

#include "nlohmann\json.hpp"

template<class T>
class serializable
{
  virtual T serialize() = 0;
  virtual void deserialize(const T& payload) = 0;
};

#define TRY_PARSE(t, j, key, out_value) try_parse<t>(j, key, out_value, __FUNCTION__) 

template<typename T>
bool try_parse(const nlohmann::json& j, const std::string& key, T& out_value, const char* function_name = nullptr)
{
  if (j.contains(key))
  {
    out_value = j[key];
    return true;
  }
  if (function_name != nullptr)
  {
    std::cout << "json try_parse key missing: " << key.c_str() << " in function " << function_name << std::endl;
  }
  else
  {
    std::cout << "json try_parse key missing: " << key.c_str() << std::endl;
  }
  return false;
}
