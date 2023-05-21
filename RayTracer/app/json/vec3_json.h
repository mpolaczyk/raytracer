#pragma once

#include <iostream>

#include "app/json/serializable.h"
#include "math/vec3.h"

class vec3_serializer : serializable<nlohmann::json>
{
public:
  static nlohmann::json serialize(const vec3& value);
  static vec3 deserialize(const nlohmann::json& j);
};

inline std::ostream& operator<<(std::ostream& out, const vec3& v) { return out << '[' << v.x << ',' << v.y << ',' << v.z << ']'; }
