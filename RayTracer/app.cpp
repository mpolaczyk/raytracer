#include "stdafx.h"

#include "app.h"

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