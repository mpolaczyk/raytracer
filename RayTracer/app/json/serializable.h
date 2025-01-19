#pragma once

// Warning!
// Don't include serializable.h outside of "*_json.h" files. "nlohmann\json.hpp" is heady to compile.

#include "nlohmann\json.hpp"

template<class T>
class serializable
{
  virtual T serialize() = 0;
  virtual void deserialize(const T& payload) = 0;
};

#define TRY_PARSE(t, j, key, out_value) try_parse<t>(j, key, out_value, __FUNCTION__) 

template<typename T>
bool try_parse(const nlohmann::json& j, const std::string& key, T& out_value, const char* function_name = nullptr);