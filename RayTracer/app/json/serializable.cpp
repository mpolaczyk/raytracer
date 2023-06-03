#include "stdafx.h"
#include "serializable.h"
#include "app/factories.h"

template<typename T>
bool try_parse(const nlohmann::json& j, const std::string& key, T& out_value, const char* function_name)
{
  if (j.contains(key))
  {
    out_value = j[key];
    return true;
  }
  if (function_name != nullptr)
  {
    logger::warn("json try_parse key missing: {0} in function {1}", key, function_name);
  }
  else
  {
    logger::warn("json try_parse key missing: {0}", key);
  }
  return false;
}

template bool try_parse<bool>(const nlohmann::json& j, const std::string& key, bool& out_value, const char* function_name);
template bool try_parse<int>(const nlohmann::json& j, const std::string& key, int& out_value, const char* function_name);
template bool try_parse<float>(const nlohmann::json& j, const std::string& key, float& out_value, const char* function_name);
template bool try_parse<int32_t>(const nlohmann::json& j, const std::string& key, int32_t& out_value, const char* function_name);

template bool try_parse<renderer_type>(const nlohmann::json& j, const std::string& key, renderer_type& out_value, const char* function_name);
template bool try_parse<hittable_type>(const nlohmann::json& j, const std::string& key, hittable_type& out_value, const char* function_name);
template bool try_parse<material_type>(const nlohmann::json& j, const std::string& key, material_type& out_value, const char* function_name);

template bool try_parse<std::string>(const nlohmann::json& j, const std::string& key, std::string& out_value, const char* function_name);
template bool try_parse<nlohmann::json>(const nlohmann::json& j, const std::string& key, nlohmann::json& out_value, const char* function_name);