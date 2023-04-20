#pragma once

#include "math/camera.h"
#include "processing/async_renderer_base.h"
#include "math/hittables.h"
#include "math/materials.h"

#include "app/json/serializable.h"

class window_config : serializable<nlohmann::json>
{
public:
  int x = 100;
  int y = 100;
  int w = 1920;
  int h = 1080;

  nlohmann::json serialize();
  void deserialize(const nlohmann::json& j);

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(window_config, x, y, w, h);  // to_json only
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
};

struct raytracer_window_model
{
  renderer_panel_model rp_model;
};

struct output_window_model
{
  float zoom = 1.0f;
  bool auto_render = false;
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
  camera_panel_model cp_model;
  new_object_panel_model nop_model;
  delete_object_panel_model d_model;
  material_selection_combo_model m_model;
};


/*
   app_state - root structure for the application
   - accessible from multiple panels/widgets
   - holds resources
   - persistent
*/
class app_instance
{
public:

  app_instance()
  {
  }
  ~app_instance()
  {
    if (renderer != nullptr)
    {
      delete renderer;
    }
    if (default_material != nullptr)
    {
      delete default_material;
    }
  }
  // Scene state
  scene scene_root;
  camera_config camera_conf;

  // Rendering state
  renderer_config renderer_conf;
  material_instances materials;
  
  // OS window state
  window_config window_conf;

  // Imgui window states
  raytracer_window_model rw_model;
  output_window_model ow_model;
  scene_editor_window_model sew_model;

  // Runtime state
  int output_width = 0;
  int output_height = 0;
  struct ID3D11ShaderResourceView* output_srv = nullptr;
  struct ID3D11Texture2D* output_texture = nullptr;
  async_renderer_base* renderer;
  material* default_material = nullptr;
  vec3 center_of_scene;
  float distance_to_center_of_scene = 0.0f;

  float output_window_lmb_x = -1.0f;
  float output_window_lmb_y = -1.0f;
  class hittable* selected_object = nullptr;

  void load_scene_state();
  void save_scene_state();
  void load_rendering_state();
  void save_rendering_state();
  void load_window_state();
  void save_window_state();
};

void draw_camera_panel(camera_panel_model& model, app_instance& state);
void draw_renderer_panel(renderer_panel_model& model, app_instance& state);
void draw_hotkeys_panel(app_instance& state);
void draw_raytracer_window(raytracer_window_model& model, app_instance& state);
void draw_output_window(output_window_model& model, app_instance& state);
void draw_scene_editor_window(scene_editor_window_model& model, app_instance& state);
void draw_new_object_panel(new_object_panel_model& model, app_instance& state);
void draw_material_selection_combo(material_selection_combo_model& model, app_instance& state);
void draw_delete_object_panel(delete_object_panel_model& model, app_instance& state);

void update_default_spawn_position(app_instance& state);