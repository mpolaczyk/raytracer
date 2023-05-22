#pragma once

#include "app/json/serializable.h"
#include "math/vec3.h"

class vec3_serializer : serializable<nlohmann::json>
{
public:
  static nlohmann::json serialize(const vec3& value);
  static vec3 deserialize(const nlohmann::json& j);
};
