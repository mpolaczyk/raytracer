#include "stdafx.h"

#include "nlohmann\json.hpp"

#include "app.h"


nlohmann::json app_state::serialize()
{
  nlohmann::json j;
  j["camera_setting"] = camera_setting.serialize();
  j["renderer_setting"] = renderer_setting.serialize();
  j["materials"] = materials.serialize();
  j["scene"] = scene_root.serialize();
  return j;
}

void app_state::deserialize(const nlohmann::json& j)
{
  camera_setting.deserialize(j["camera_setting"]);
  renderer_setting.deserialize(j["renderer_setting"]);
  materials.deserialize(j["materials"]);
  scene_root.deserialize(j["scene"]);
}