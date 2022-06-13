// Inspired by https://raytracing.github.io/books/RayTracingInOneWeekend.html

#include "stdafx.h"

#include "frame_renderer.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"
#include "common.h"

// Textures
solid_texture t_g(grey);
solid_texture t_ggg(grey*1.5f);
solid_texture t_sky(vec3(0.53f, 0.81f, 0.92f)*0.2f);
solid_texture t_lightbulb(vec3(4.0f, 4.0f, 4.0f)*0.5f);
checker_texture t_ch(&t_g, &t_ggg);
// Materials
diffuse_material white_blue_diffuse(white_blue);
diffuse_material green_diffuse(green);
diffuse_material yellow_diffuse(yellow);
diffuse_material red_diffuse(red);
metal_material metal_shiny(grey, 0.0f);
metal_material metal_matt(grey, 0.02f);
dialectric_material glass(2.8f);
texture_material world_base(&t_ch);
// Lights
diffuse_light_material diff_light_strong = diffuse_light_material(&t_lightbulb);
diffuse_light_material diff_light_weak = diffuse_light_material(&t_sky);

// Camera settings
vec3 look_from = vec3(0.0f, 1.0f, -1.0f);
vec3 look_at = vec3(0.0f, 0.0f, 0.0f);
float field_of_view = 80.0f;
float aspect_ratio = 16.0f / 9.0f;
float aperture = 0.02f;

// Scene ideas:
// - light bend by a sphere, focus in a spot
// - light bend by a prism (to implement: index of refraction depends on the wave length(color))
// - green light focused by sphere is mixed with red light focused by a sphere, both produce yellow
// - light reflected of a diffuse object seen on a different object
// - reflections of an object that is not on the screen
// - reflections of all above in a bigger mirror

void draw_scene_massive(sphere_list& world)
{
  look_from = vec3(0.0f, 3.7f, 5.0f);
  look_at = vec3(0.0f, 0.0f, 2.2f);

  std::vector<material*> materials;
  materials.push_back(&red_diffuse);
  materials.push_back(&glass);
  materials.push_back(&green_diffuse);
  materials.push_back(&metal_matt);
  materials.push_back(&white_blue_diffuse);
  materials.push_back(&yellow_diffuse);
  materials.push_back(&metal_shiny);
  int size_x = 12;
  int size_z = 12;
  for (int x = 0; x < size_x; x++)
  {
    for (int z = 0; z < size_z; z++)
    {
      float pos_offset = size_x / 2.0f;
      int material_id = (x * 2 + z * 5) % (materials.size() - 1);
      sphere* obj = new sphere(vec3((float)x - pos_offset, 0.0f, (float)z - pos_offset), 0.4f, materials[material_id]);
      world.add(obj);
    }
  }
  sphere* a = new sphere(vec3(-2.f, 2.0f, 1.0f), 0.5f, &glass);
  sphere* b = new sphere(vec3(0.f, 2.0f, 1.0f), 0.5f, &glass);
  sphere* c = new sphere(vec3(2.f, 2.0f, 1.0f), 0.5f, &glass);
  sphere* d = new sphere(vec3(-2.f, 2.0f, 2.5f), 0.5f, &glass);
  sphere* e = new sphere(vec3(0.f, 2.0f, 2.5f), 0.5f, &glass);
  sphere* f = new sphere(vec3(2.f, 2.0f, 2.5f), 0.5f, &glass);
  sphere* g = new sphere(vec3(-2.f, 2.0f, 4.0f), 0.5f, &glass);
  sphere* h = new sphere(vec3(0.f, 2.0f, 4.0f), 0.5f, &glass);
  sphere* i = new sphere(vec3(2.f, 2.0f, 4.0f), 0.5f, &glass);
  world.add(a); world.add(b); world.add(c); world.add(d); world.add(e); world.add(f); world.add(g); world.add(h); world.add(i);
  sphere* j = new sphere(vec3(0.f, -100.5f, -1.f), 100.f, &world_base);
  world.add(j);
}

void draw_scene_lights(sphere_list& world)
{
  look_from = vec3(0.0f, 0.3f, 0.0f);
  look_at = vec3(0.0f, 0.2f, -1.0f);
  
  // Ignore leaks for now, it does not matter.
  sphere* a = new sphere(vec3(1.0f, 0.4f, -1.4f), 0.4f, &metal_shiny);
  sphere* c = new sphere(vec3(-1.0f, 0.4f, -1.4f), 0.4f, &metal_shiny);
  sphere* b = new sphere(vec3(0.f, 0.0f, -1.2f), 0.4f, &glass);
  sphere* d = new sphere(vec3(0.3f, 0.4f, -1.4f), 0.2f, &green_diffuse);
  sphere* g = new sphere(vec3(-0.3f, 0.4f, -1.4f), 0.2f, &red_diffuse);
  sphere* e = new sphere(vec3(0.f, -100.5f, -1.f), 100.f, &world_base);
  sphere* f = new sphere(vec3(0.f, 0.9f, -1.f), 0.4f, &diff_light_strong);
  sphere* h = new sphere(vec3(0.f, 0.0f, 0.f), 100.0f, &diff_light_weak);
  world.add(a); world.add(b); world.add(c); world.add(d); world.add(e); world.add(f); world.add(g); world.add(h);
}

int main()
{
  random_cache::init();

  sphere_list world;
  //draw_scene_massive(world);
  draw_scene_lights(world);

  world.build_boxes();

  float dist_to_focus = (look_from - look_at).length();

  int resolution_vertical = 1080;
  int resolution_horizontal = (int)((float)resolution_vertical) * aspect_ratio;

  std::vector<std::pair<uint32_t, camera_setup>> camera_states;
  camera_setup state0 = camera_setup(look_from, look_at, field_of_view, aspect_ratio, aperture, dist_to_focus, 0.2f);
  camera_setup state1 = camera_setup(look_from, look_at, field_of_view, aspect_ratio, aperture, dist_to_focus, 1.0f);
  camera_setup state2 = camera_setup(look_from, look_at, field_of_view, aspect_ratio, aperture, dist_to_focus, 0.0f);

  camera_states.push_back(std::make_pair(0, state0));
  camera_states.push_back(std::make_pair(10, state1));
  camera_states.push_back(std::make_pair(20, state2));

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
