// Inspired by https://raytracing.github.io/books/RayTracingInOneWeekend.html

#include "stdafx.h"

#include "frame_renderer.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"
#include "common.h"

int main()
{
  random_cache::init();
  
  diffuse_material white_diffuse(white_blue);
  diffuse_material green_diffuse(green);
  diffuse_material yellow_diffuse(yellow);
  diffuse_material red_diffuse(red);
  metal_material metal_shiny(grey, 0.0f);
  metal_material metal_matt(grey, 0.02f);
  dialectric_material glass(1.5f);

  solid_color cha = solid_color(0.2, 0.3, 0.1);
  solid_color chb = solid_color(0.9, 0.9, 0.9);
  checker_texture ch = checker_texture(&cha, &chb);
  texture_material world_base(&ch);

  vec3 look_from = vec3(0.0f, 3.7f, 5.0f);
  vec3 look_at = vec3(0.0f, 0.0f, 2.2f);
  float field_of_view = 90.0f;
  float aspect_ratio = 16.0f / 9.0f;
  float aperture = 0.05f;
  float dist_to_focus = (look_from - look_at).length();

  int resolution_vertical = 1080;
  int resolution_horizontal = (int)((float)resolution_vertical) * aspect_ratio;

  std::vector<std::pair<uint32_t, camera_setup>> camera_states;
  camera_setup state0 = camera_setup(look_from, look_at, field_of_view, aspect_ratio, aperture, dist_to_focus, 0.0f);
  camera_setup state1 = camera_setup(look_from, look_at, field_of_view, aspect_ratio, aperture, dist_to_focus, 1.0f);
  camera_setup state2 = camera_setup(look_from, look_at, field_of_view, aspect_ratio, aperture, dist_to_focus, 0.0f);

  camera_states.push_back(std::make_pair(0, state0));
  camera_states.push_back(std::make_pair(10, state1));
  camera_states.push_back(std::make_pair(20, state2));

  sphere_list world;
  std::vector<material*> materials;
  materials.push_back(&red_diffuse);
  materials.push_back(&glass);
  materials.push_back(&green_diffuse);
  materials.push_back(&metal_matt);
  materials.push_back(&white_diffuse);
  materials.push_back(&yellow_diffuse);
  materials.push_back(&metal_shiny);
  int size_x = 12;
  int size_z = 12;
  for (int x = 0; x < size_x; x++)
  {
    for (int z = 0; z < size_z; z++)
    {
      float pos_offset = size_x / 2.0f;
      int material_id = (x*2 + z*5) % (materials.size() - 1);
      sphere obj(vec3((float)x - pos_offset, 0.0f, (float)z- pos_offset), 0.4f, materials[material_id]);
      world.add(obj);
    }
  }
  world.add(sphere(vec3(-2.f, 2.0f, 1.0f), 0.5f, &glass));
  world.add(sphere(vec3( 0.f, 2.0f, 1.0f), 0.5f, &glass));
  world.add(sphere(vec3( 2.f, 2.0f, 1.0f), 0.5f, &glass));
  world.add(sphere(vec3(-2.f, 2.0f, 2.5f), 0.5f, &glass));
  world.add(sphere(vec3( 0.f, 2.0f, 2.5f), 0.5f, &glass));
  world.add(sphere(vec3( 2.f, 2.0f, 2.5f), 0.5f, &glass));
  world.add(sphere(vec3(-2.f, 2.0f, 4.0f), 0.5f, &glass));
  world.add(sphere(vec3( 0.f, 2.0f, 4.0f), 0.5f, &glass));
  world.add(sphere(vec3( 2.f, 2.0f, 4.0f), 0.5f, &glass));

  world.add(sphere(vec3(0.f, -100.5f, -1.f), 100.f, &world_base));
  world.build_boxes();

  frame_renderer renderer = frame_renderer(resolution_horizontal, resolution_vertical, renderer_settings::medium_quality_preset);

  if (true)
  {
    renderer.render_single(world, state0);
  }
  else
  {
    renderer.render_multiple(world, camera_states);
  }
  
  return 0;
}
