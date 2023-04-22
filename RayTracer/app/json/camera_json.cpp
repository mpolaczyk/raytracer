#include "stdafx.h"

#include "math/camera.h"


nlohmann::json camera_config::serialize()
{
  nlohmann::json j;
  to_json(j, *this);
  j["look_from"] = vec3_serializer::serialize(look_from);
  j["look_dir"] = vec3_serializer::serialize(look_dir);
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

  nlohmann::json jlook_dir;
  if (TRY_PARSE(nlohmann::json, j, "look_dir", jlook_dir)) { look_dir = vec3_serializer::deserialize(jlook_dir); }

  nlohmann::json jlook_from;
  if (TRY_PARSE(nlohmann::json, j, "look_from", jlook_from)) { look_from = vec3_serializer::deserialize(jlook_from); }
}