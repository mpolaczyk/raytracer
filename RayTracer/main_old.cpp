// Inspired by https://raytracing.github.io/books/RayTracingInOneWeekend.html

#include "stdafx.h"

#include "frame_renderer.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"
#include "common.h"

// Textures
solid_texture t_g(grey);
solid_texture t_ggg(grey * 1.5f);
solid_texture t_sky(white * 0.4f);
solid_texture t_lightbulb(vec3(2.0f, 2.0f, 2.0f));
solid_texture t_lightbulb_strong(vec3(8.0f, 8.0f, 8.0f));
solid_texture t_lightbulb_ultra_strong(vec3(15.0f, 15.0f, 15.0f));
checker_texture t_ch(&t_g, &t_ggg);
// Materials
diffuse_material white_blue_diffuse(white_blue);
diffuse_material white_diffuse(white);
diffuse_material green_diffuse(green);
diffuse_material yellow_diffuse(yellow);
diffuse_material red_diffuse(red);
metal_material metal_shiny(grey, 0.0f);
metal_material metal_matt(grey, 0.02f);
dialectric_material glass(1.5f);
texture_material world_base(&t_ch);
// Lights
diffuse_light_material diff_light = diffuse_light_material(&t_lightbulb);
diffuse_light_material diff_light_strong = diffuse_light_material(&t_lightbulb_strong);
diffuse_light_material diff_light_ultra_strong = diffuse_light_material(&t_lightbulb_ultra_strong);
diffuse_light_material diff_light_sky = diffuse_light_material(&t_sky);

// Camera settings
int resolution_vertical = 1080;
vec3 look_from = vec3(0.0f, 1.0f, -1.0f);
vec3 look_at = vec3(0.0f, 0.0f, 0.0f);
float field_of_view = 80.0f;
float aspect_ratio = 16.0f / 9.0f;
float aperture = 0.02f;



void draw_scene_massive(hittable_list& world)
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

void draw_scene_lights(hittable_list& world)
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
  sphere* f = new sphere(vec3(0.f, 0.9f, -1.f), 0.4f, &diff_light);
  sphere* h = new sphere(vec3(0.f, 0.0f, 0.f), 100.0f, &diff_light_sky);
  world.add(a); world.add(b); world.add(c); world.add(d); world.add(e); world.add(f); world.add(g); world.add(h);
}

void draw_scene_box(hittable_list& world)
{
  resolution_vertical = 600;
  look_from = vec3(278, 278, -800);
  look_at = vec3(278, 278, 0);
  field_of_view = 40.0f;
  aspect_ratio = 1.0f;

  yz_rect* r1 = new yz_rect(0, 555, 0, 555, 555, &green_diffuse);
  yz_rect* r2 = new yz_rect(0, 555, 0, 555, 0, &red_diffuse);
  xz_rect* r3 = new xz_rect(213, 343, 127, 332, 554, &diff_light_ultra_strong);
  xz_rect* r4 = new xz_rect(0, 555, 0, 555, 0, &white_diffuse);
  xz_rect* r5 = new xz_rect(0, 555, 0, 555, 555, &white_diffuse);
  xy_rect* r6 = new xy_rect(0, 555, 0, 555, 555, &white_diffuse);
  world.add(r1); world.add(r2); world.add(r3); world.add(r4); world.add(r5); world.add(r6);

  sphere* e1 = new sphere(vec3(230.0f, 290.0f, 250.f), 120.f, &glass);
  world.add(e1);
  sphere* e3 = new sphere(vec3(270.0f, 50.0f, 210.f), 30.f, &metal_shiny);
  world.add(e3);

  sphere* e2 = new sphere(vec3(270.0f, 270.0f, 250.f), 1100.f, &diff_light_sky);
  world.add(e2);
}

//int main_old()
//{
//  random_cache::init();
//
//  hittable_list world;
//  //draw_scene_massive(world);
//  //draw_scene_lights(world);
//  draw_scene_box(world);
//
//  world.build_boxes();
//
//  float dist_to_focus = (look_from - look_at).length();
//
//  int resolution_horizontal = (int)((float)resolution_vertical) * aspect_ratio;
//
//  std::vector<std::pair<uint32_t, camera_config>> camera_states;
//  camera_config state0 = camera_config(look_from, look_at, field_of_view, aspect_ratio, aperture, dist_to_focus, 0.0f);
//  camera_config state1 = camera_config(look_from, look_at, field_of_view, aspect_ratio, aperture, dist_to_focus, 1.0f);
//  camera_config state2 = camera_config(look_from, look_at, field_of_view, aspect_ratio, aperture, dist_to_focus, 0.0f);
//
//  camera_states.push_back(std::make_pair(0, state0));
//  camera_states.push_back(std::make_pair(10, state1));
//  camera_states.push_back(std::make_pair(20, state2));
//
//  frame_renderer renderer;
//  renderer.set_config(resolution_horizontal, resolution_vertical, renderer_config::ultra_high_quality_preset);
//
//  if (true)
//  {
//    renderer.render_single(world, state0);
//  }
//  else
//  {
//    renderer.render_multiple(world, camera_states);
//  }
//
//  return 0;
//}
