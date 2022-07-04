#include "stdafx.h"

#include "vec3.h"


nlohmann::json vec3::serialize()
{
  nlohmann::json j;
  to_json(j, *this);
  return j;
}

void vec3::deserialize(const nlohmann::json& j)
{
  TRY_PARSE(float, j, "x", x);
  TRY_PARSE(float, j, "y", y);
  TRY_PARSE(float, j, "z", z);
}