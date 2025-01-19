#pragma once

#include "camera.h"
#include "frame_renderer.h"
#include "hittables.h"

/*
   app_state - root structure for the application
   - accessible from multiple panels/widgets
   - holds resources
   - persistent (in future)
*/
struct app_state
{
  // Initial state
  camera_config camera_setting;
  renderer_config renderer_setting;
  hittable_list world;
  int resolution_vertical = 0;
  int resolution_horizontal = 0;
  float background_color[3] = { 0,0,0 };

  // Runtime state
  int output_width = 0;
  int output_height = 0;
  class ID3D11ShaderResourceView* output_srv = nullptr;
  class ID3D11Texture2D* output_texture = nullptr;
  frame_renderer renderer;
  material* default_material = nullptr;
};

/*
 *_model - for each panel/window
 - not part of app state
 - owned by UI
 - required to maintain panel/window state between frames
 - not needed to be shared between multiple panels/widgets
 - not persistent
*/
struct camera_panel_model
{
  bool use_custom_focus_distance = false;
};

struct renderer_panel_model
{

};

struct raytracer_window_model
{
  camera_panel_model cp_model;
  renderer_panel_model rp_model;
};

struct output_window_model
{
  float zoom = 1.0f;
  bool real_time_update = true;
};

struct new_object_panel_model
{
  int selected_type = 0;
  hittable* hittable = nullptr;
};

struct scene_editor_window_model
{
  int selected_id = -1;
  new_object_panel_model nop_model;
};

void draw_camera_panel(camera_panel_model& model, app_state& state);
void draw_renderer_panel(renderer_panel_model& model, app_state& state);
void draw_raytracer_window(raytracer_window_model& model, app_state& state);
void draw_output_window(output_window_model& model, app_state& state);
void draw_scene_editor_window(scene_editor_window_model& model, app_state& state);
void draw_new_object_panel(new_object_panel_model& model, app_state& state);
