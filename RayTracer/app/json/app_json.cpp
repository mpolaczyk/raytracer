#include "stdafx.h"

#include <filesystem>
#include <fstream>
#include <system_error>

#include "app/json/app_json.h"
#include "app/json/vec3_json.h"
#include "app/json/frame_renderer_json.h"
#include "app/json/materials_json.h"
#include "app/json/hittables_json.h"
#include "math/hittables.h"

nlohmann::json window_config_serializer::serialize(const window_config& value)
{
  nlohmann::json j;
  j["x"] = value.x;
  j["y"] = value.y;
  j["w"] = value.w;
  j["h"] = value.h;
  return j;
}

window_config window_config_serializer::deserialize(const nlohmann::json& j)
{
  window_config value;
  TRY_PARSE(int, j, "x", value.x);
  TRY_PARSE(int, j, "y", value.y);
  TRY_PARSE(int, j, "w", value.w);
  TRY_PARSE(int, j, "h", value.h);
  return value;
}

nlohmann::json camera_config_serializer::serialize(const camera_config& value)
{
  nlohmann::json j;
  j["field_of_view"] = value.field_of_view;
  j["aspect_ratio_h"] = value.aspect_ratio_h;
  j["aspect_ratio_w"] = value.aspect_ratio_w;
  j["aperture"] = value.aperture;
  j["dist_to_focus"] = value.dist_to_focus;
  j["type"] = value.type;
  j["look_from"] = vec3_serializer::serialize(value.look_from);
  j["look_dir"] = vec3_serializer::serialize(value.look_dir);
  return j;
}

camera_config camera_config_serializer::deserialize(const nlohmann::json& j)
{
  camera_config value;
  TRY_PARSE(float, j, "field_of_view", value.field_of_view);
  TRY_PARSE(float, j, "aspect_ratio_h", value.aspect_ratio_h);
  TRY_PARSE(float, j, "aspect_ratio_w", value.aspect_ratio_w);
  TRY_PARSE(float, j, "aperture", value.aperture);
  TRY_PARSE(float, j, "dist_to_focus", value.dist_to_focus);
  TRY_PARSE(float, j, "type", value.type);

  nlohmann::json jlook_dir;
  if (TRY_PARSE(nlohmann::json, j, "look_dir", jlook_dir)) { value.look_dir = vec3_serializer::deserialize(jlook_dir); }

  nlohmann::json jlook_from;
  if (TRY_PARSE(nlohmann::json, j, "look_from", jlook_from)) { value.look_from = vec3_serializer::deserialize(jlook_from); }

  return value;
}

void app_instance::load_scene_state()
{
  const std::string scene_path = io::get_scene_file_path();
  std::ifstream input_stream(scene_path.c_str());
  if (!input_stream.is_open())
  {
    logger::warn("Unable to open scene file: {0}", scene_path);
    return;
  }
  nlohmann::json j;
  try
  {
    input_stream >> j;
  }
  catch (const std::exception& ex)
  {
    logger::error("Unable to parse scene file: {0}. Error: {1}", scene_path, ex.what());
    return;
  }

  nlohmann::json jcamera_conf;
  if (TRY_PARSE(nlohmann::json, j, "camera_config", jcamera_conf)) { *camera_conf = camera_config_serializer::deserialize(jcamera_conf); }

  nlohmann::json jscene_root;
  if (TRY_PARSE(nlohmann::json, j, "scene", jscene_root))
  {
    // Deserialize into a temporary scene to avoid partial updates if deserialization fails.
    scene temp_scene;
    try
    {
      scene_serializer::deserialize(jscene_root, &temp_scene);
    }
    catch (const std::exception& ex)
    {
      logger::error("Unable to deserialize scene data: {0}", ex.what());
      return;
    }
    scene_root->objects.swap(temp_scene.objects);
    // Swap transfers ownership efficiently; temp_scene will clean up prior objects on destruction.
    int scene_id = scene_root->id;
    hittable_type scene_type = scene_root->type;
    std::string scene_material_id = scene_root->material_id;
    TRY_PARSE(int, jscene_root, "id", scene_id);
    TRY_PARSE(hittable_type, jscene_root, "type", scene_type);
    TRY_PARSE(std::string, jscene_root, "material_id", scene_material_id);
    scene_root->id = scene_id;
    scene_root->type = scene_type;
    scene_root->material_id = scene_material_id;
    // Reset runtime selection pointer and scene editor indices (-1 means no selection).
    selected_object = nullptr;
    sew_model.selected_id = -1;
    sew_model.d_model.selected_id = -1;
  }

  input_stream.close();
}

void app_instance::load_rendering_state()
{
  std::ifstream input_stream(io::get_rendering_file_path().c_str());
  nlohmann::json j;
  input_stream >> j;

  nlohmann::json jrenderer_conf;
  if (TRY_PARSE(nlohmann::json, j, "renderer_config", jrenderer_conf)) { *renderer_conf = renderer_config_serializer::deserialize(jrenderer_conf); }

  nlohmann::json jmaterials;
  if (TRY_PARSE(nlohmann::json, j, "materials", jmaterials)) { *materials = material_instances_serializer::deserialize(jmaterials); }

  input_stream.close();
}

void app_instance::load_window_state()
{
  std::ifstream input_stream(io::get_window_file_path().c_str());
  nlohmann::json j;
  input_stream >> j;

  nlohmann::json jwindow;
  if (TRY_PARSE(nlohmann::json, j, "window", jwindow)) { window_conf = window_config_serializer::deserialize(jwindow); }

  TRY_PARSE(bool, j, "auto_render", ow_model.auto_render);
  TRY_PARSE(float, j, "zoom", ow_model.zoom);
  
  input_stream.close();
}


void app_instance::save_scene_state()
{
  nlohmann::json j;
  j["camera_config"] = camera_config_serializer::serialize(*camera_conf);
  j["scene"] = scene_serializer::serialize(scene_root);
  std::ofstream output_stream(io::get_scene_file_path().c_str(), std::ios_base::out | std::ios::binary);
  std::string str = j.dump(2);
  if (output_stream.is_open())
  {
    output_stream.write(str.data(), str.length());
  }
  output_stream.close();
  sync_scene_file_timestamp();
}

bool app_instance::reload_scene_state_if_changed()
{
  std::error_code error;
  const std::string scene_path = io::get_scene_file_path();
  std::filesystem::file_time_type time = std::filesystem::last_write_time(scene_path, error);
  if (error)
  {
    return false;
  }
  if (time == scene_file_last_write_time)
  {
    return false;
  }
  scene_file_last_write_time = time;
  load_scene_state();
}

void app_instance::sync_scene_file_timestamp()
{
  std::error_code error;
  const std::string scene_path = io::get_scene_file_path();
  std::filesystem::file_time_type time = std::filesystem::last_write_time(scene_path, error);
  if (error)
  {
    return;
  }
  scene_file_last_write_time = time;
}

void app_instance::save_rendering_state()
{
  nlohmann::json j;
  j["renderer_config"] = renderer_config_serializer::serialize(*renderer_conf);
  j["materials"] = material_instances_serializer::serialize(*materials);
  std::ofstream o(io::get_rendering_file_path().c_str(), std::ios_base::out | std::ios::binary);
  std::string str = j.dump(2);
  if (o.is_open())
  {
    o.write(str.data(), str.length());
  }
  o.close();
}

void app_instance::save_window_state()
{
  nlohmann::json j;
  j["window"] = window_config_serializer::serialize(window_conf);
  j["auto_render"] = ow_model.auto_render;
  j["zoom"] = ow_model.zoom;
  std::ofstream o(io::get_window_file_path().c_str(), std::ios_base::out | std::ios::binary);
  std::string str = j.dump(2);
  if (o.is_open())
  {
    o.write(str.data(), str.length());
  }
  o.close();
}
