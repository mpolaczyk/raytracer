#pragma once

#include "camera.h"
#include "frame_renderer.h"
#include "hittables.h"
#include "materials.h"

#include "nlohmann\json.hpp"
#include "serializable.h"

/*
   app_state - root structure for the application
   - accessible from multiple panels/widgets
   - holds resources
   - persistent
*/
class app_state : serializable<nlohmann::json>
{
public:
  // Initial state
  camera_config camera_setting;
  renderer_config renderer_setting;
  hittable_list world;
  int resolution_vertical = 0;
  material_instances materials;

  // Runtime state
  int resolution_horizontal = 0;
  int output_width = 0;
  int output_height = 0;
  struct ID3D11ShaderResourceView* output_srv = nullptr;
  struct ID3D11Texture2D* output_texture = nullptr;
  bool output_force_recreate = true;
  frame_renderer renderer;
  material* default_material = nullptr;
  vec3 center_of_scene;
  float distance_to_center_of_scene = 0.0f;

  nlohmann::json serialize();
  void deserialize(const nlohmann::json& j);

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(app_state, resolution_vertical, resolution_horizontal);

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
  bool render_pressed = false;
  float background_color[3] = { 1.0f,1.0f,1.0f };
};

struct raytracer_window_model
{
  camera_panel_model cp_model;
  renderer_panel_model rp_model;
};

struct output_window_model
{
  float zoom = 1.0f;
  bool auto_refresh = false;
};

struct material_selection_combo_model
{
  int selected_material_name_index = 0;
};

struct new_object_panel_model
{
  int selected_type = 0;
  hittable* hittable = nullptr;
  material_selection_combo_model m_model;
};

struct delete_object_panel_model
{
  int selected_id = 0;
};

struct scene_editor_window_model
{
  int selected_id = -1;
  new_object_panel_model nop_model;
  delete_object_panel_model d_model;
  material_selection_combo_model m_model;
};

void draw_camera_panel(camera_panel_model& model, app_state& state);
void draw_renderer_panel(renderer_panel_model& model, app_state& state);
void draw_raytracer_window(raytracer_window_model& model, app_state& state);
void draw_output_window(output_window_model& model, app_state& state);
void draw_scene_editor_window(scene_editor_window_model& model, app_state& state);
void draw_new_object_panel(new_object_panel_model& model, app_state& state);
void draw_material_selection_combo(material_selection_combo_model& model, app_state& state);
void draw_delete_object_panel(delete_object_panel_model& model, app_state& state);

void update_default_spawn_position(app_state& state);

void load_app_state(app_state& out_state);
void save_app_state(app_state& in_state);