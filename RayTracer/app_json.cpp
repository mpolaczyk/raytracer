#include "stdafx.h"

#include <fstream>

#include "app.h"


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


void app_state::load_scene_state()
{
  std::ifstream input_stream("scene.json");
  nlohmann::json j;
  input_stream >> j;

  nlohmann::json jcamera_setting;
  if (TRY_PARSE(nlohmann::json, j, "camera_setting", jcamera_setting)) { camera_setting.deserialize(jcamera_setting); }

  nlohmann::json jscene_root;
  if (TRY_PARSE(nlohmann::json, j, "scene", jscene_root)) { scene_root.deserialize(jscene_root); }

  input_stream.close();
}

void app_state::load_rendering_state()
{
  std::ifstream input_stream("rendering.json");
  nlohmann::json j;
  input_stream >> j;

  nlohmann::json jrenderer_setting;
  if (TRY_PARSE(nlohmann::json, j, "renderer_setting", jrenderer_setting)) { renderer_setting.deserialize(jrenderer_setting); }

  nlohmann::json jmaterials;
  if (TRY_PARSE(nlohmann::json, j, "materials", jmaterials)) { materials.deserialize(jmaterials); }

  input_stream.close();
}

void app_state::load_window_state()
{
  std::ifstream input_stream("window.json");
  nlohmann::json j;
  input_stream >> j;

  nlohmann::json jwindow;
  if (TRY_PARSE(nlohmann::json, j, "window", jwindow)) { window.deserialize(jwindow); }

  TRY_PARSE(bool, j, "auto_render", ow_model.auto_render);
  TRY_PARSE(float, j, "zoom", ow_model.zoom);
  
  input_stream.close();
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

void app_state::save_window_state()
{
  nlohmann::json j;
  j["window"] = window.serialize();
  j["auto_render"] = ow_model.auto_render;
  j["zoom"] = ow_model.zoom;
  std::ofstream o("window.json", std::ios_base::out | std::ios::binary);
  std::string str = j.dump(2);
  if (o.is_open())
  {
    o.write(str.data(), str.length());
  }
  o.close();
}
