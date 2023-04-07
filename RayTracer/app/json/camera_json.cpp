#include "stdafx.h"

#include "math/camera.h"


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
  TRY_PARSE(float, j, "field_of_view", field_of_view);
  TRY_PARSE(float, j, "aspect_ratio_h", aspect_ratio_h);
  TRY_PARSE(float, j, "aspect_ratio_w", aspect_ratio_w);
  TRY_PARSE(float, j, "aperture", aperture);
  TRY_PARSE(float, j, "dist_to_focus", dist_to_focus);
  TRY_PARSE(float, j, "type", type);

  nlohmann::json jlook_at;
  if (TRY_PARSE(nlohmann::json, j, "look_at", jlook_at)) { look_at.deserialize(jlook_at); }

  nlohmann::json jlook_from;
  if (TRY_PARSE(nlohmann::json, j, "look_from", jlook_from)) { look_from.deserialize(jlook_from); }
}