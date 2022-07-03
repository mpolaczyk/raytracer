#include "stdafx.h"

#include "nlohmann\json.hpp"

#include "app.h"


nlohmann::json app_state::serialize()
{
  nlohmann::json j;
  to_json(j, *this);
  j["camera_setting"] = camera_setting.serialize();
  j["renderer_setting"] = renderer_setting.serialize();
  j["materials"] = materials.serialize();
  j["world"] = world.serialize();
  return j;
}

void app_state::deserialize(const nlohmann::json& j)
{
  from_json(j, *this);
  camera_setting.deserialize(j["camera_setting"]);
  renderer_setting.deserialize(j["renderer_setting"]);
  materials.deserialize(j["materials"]);
  world.deserialize(j["world"]);
}