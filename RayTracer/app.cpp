#include "stdafx.h"

#include "app.h"

#include "nlohmann\json.hpp"
#include <fstream>

void app_state::load_scene_state()
{
  std::ifstream i("scene.json");
  nlohmann::json j;
  i >> j;
  camera_setting.deserialize(j["camera_setting"]);
  scene_root.deserialize(j["scene"]);
  i.close();
}

void app_state::save_scene_state()
{
  nlohmann::json j;
  j["camera_setting"] = camera_setting.serialize();
  j["scene"] = scene_root.serialize();
  std::ofstream o("scene.json", std::ios_base::out | std::ios::binary);
  std::string str = j.dump(2);
  if (o.is_open())
  {
    o.write(str.data(), str.length());
  }
  o.close();
}

void app_state::load_rendering_state()
{
  std::ifstream i("rendering.json");
  nlohmann::json j;
  i >> j;
  renderer_setting.deserialize(j["renderer_setting"]);
  materials.deserialize(j["materials"]);
  i.close();
}

void app_state::save_rendering_state()
{
  nlohmann::json j;
  j["renderer_setting"] = renderer_setting.serialize();
  j["materials"] = materials.serialize();
  std::ofstream o("rendering.json", std::ios_base::out | std::ios::binary);
  std::string str = j.dump(2);
  if (o.is_open())
  {
    o.write(str.data(), str.length());
  }
  o.close();
}

void update_default_spawn_position(app_state& state)
{
  // Find center of the scene, new objects scan be spawned there
  vec3 look_from = state.camera_setting.look_from;
  vec3 look_at = state.camera_setting.look_at;
  float dist_to_focus = state.camera_setting.dist_to_focus;
  // Ray to the look at position to find non colliding spawn point
  ray center_of_scene_ray(look_from, look_at - look_from);
  hit_record center_of_scene_hit;
  if (state.scene_root.hit(center_of_scene_ray, 0.01f, dist_to_focus, center_of_scene_hit))
  {
    state.center_of_scene = center_of_scene_hit.p;
    state.distance_to_center_of_scene = (center_of_scene_hit.p - look_from).length();
  }
  else
  {
    state.center_of_scene = look_at;
    state.distance_to_center_of_scene = (look_at - look_from).length();
  }
}