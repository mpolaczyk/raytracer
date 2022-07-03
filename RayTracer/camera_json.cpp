#include "stdafx.h"

#include "nlohmann\json.hpp"

#include "camera.h"


nlohmann::json camera_config::serialize()
{
  nlohmann::json j;
  to_json(j, *this);
  j["look_from"] = look_from.serialize();
  j["look_at"] = look_at.serialize();
  return j;
}

void camera_config::deserialize(const nlohmann::json& j)
{
  from_json(j, *this);
  look_from.deserialize(j["look_from"]);
  look_at.deserialize(j["look_at"]);
}