#include "stdafx.h"

#include "math/vec3.h"


nlohmann::json vec3_serializer::serialize(const vec3& value)
{
  nlohmann::json j;
  j["x"] = value.x;
  j["y"] = value.y;
  j["z"] = value.z;
  return j;
}

vec3 vec3_serializer::deserialize(const nlohmann::json& j)
{
  vec3 value;
  TRY_PARSE(float, j, "x", value.x);
  TRY_PARSE(float, j, "y", value.y);
  TRY_PARSE(float, j, "z", value.z);
  return value;
}