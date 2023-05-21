#include "stdafx.h"

#include <fstream>

#include "app/app.h"


nlohmann::json window_config::serialize()
{
  nlohmann::json j;
  to_json(j, *this);
  return j;
}

void window_config::deserialize(const nlohmann::json& j)
{
  TRY_PARSE(int, j, "x", x);
  TRY_PARSE(int, j, "y", y);
  TRY_PARSE(int, j, "w", w);
  TRY_PARSE(int, j, "h", h);
}


void app_instance::load_scene_state()
{
  std::ifstream input_stream(io::get_scene_file_path().c_str());
  nlohmann::json j;
  input_stream >> j;

  nlohmann::json jcamera_conf;
  if (TRY_PARSE(nlohmann::json, j, "camera_config", jcamera_conf)) { camera_conf.deserialize(jcamera_conf); }

  nlohmann::json jscene_root;
  if (TRY_PARSE(nlohmann::json, j, "scene", jscene_root)) { scene_root.deserialize(jscene_root); }

  input_stream.close();
}

void app_instance::load_rendering_state()
{
  std::ifstream input_stream(io::get_rendering_file_path().c_str());
  nlohmann::json j;
  input_stream >> j;

  nlohmann::json jrenderer_conf;
  if (TRY_PARSE(nlohmann::json, j, "renderer_config", jrenderer_conf)) { renderer_conf.deserialize(jrenderer_conf); }

  nlohmann::json jmaterials;
  if (TRY_PARSE(nlohmann::json, j, "materials", jmaterials)) { materials.deserialize(jmaterials); }

  input_stream.close();
}

void app_instance::load_window_state()
{
  std::ifstream input_stream(io::get_window_file_path().c_str());
  nlohmann::json j;
  input_stream >> j;

  nlohmann::json jwindow;
  if (TRY_PARSE(nlohmann::json, j, "window", jwindow)) { window_conf.deserialize(jwindow); }

  TRY_PARSE(bool, j, "auto_render", ow_model.auto_render);
  TRY_PARSE(float, j, "zoom", ow_model.zoom);
  
  input_stream.close();
}


void app_instance::save_scene_state()
{
  nlohmann::json j;
  j["camera_config"] = camera_conf.serialize();
  j["scene"] = scene_root.serialize();
  std::ofstream o(io::get_scene_file_path().c_str(), std::ios_base::out | std::ios::binary);
  std::string str = j.dump(2);
  if (o.is_open())
  {
    o.write(str.data(), str.length());
  }
  o.close();
}

void app_instance::save_rendering_state()
{
  nlohmann::json j;
  j["renderer_config"] = renderer_conf.serialize();
  j["materials"] = materials.serialize();
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
  j["window"] = window_conf.serialize();
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
