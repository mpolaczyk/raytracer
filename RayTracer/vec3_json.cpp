#include "stdafx.h"

#include "nlohmann\json.hpp"

#include "vec3.h"


nlohmann::json vec3::serialize()
{
  nlohmann::json j;
  to_json(j, *this);
  return j;
}

void vec3::deserialize(const nlohmann::json& j)
{
  from_json(j, *this);
}