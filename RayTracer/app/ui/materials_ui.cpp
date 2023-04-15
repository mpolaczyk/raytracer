#include "stdafx.h"

#include <iosfwd>

#include "math/materials.h"
#include "imgui.h"

bool material_instances::is_id_in_use(const std::string& id) const
{
  if (id.size() == 0) return true;
  auto obj = registry.find(id);
  return obj != registry.end();
}

bool material_instances::try_add(material* instance)
{
  if (instance == nullptr) return false;
  if (instance->id.size() == 0) return false;
  auto obj = registry.find(instance->id);
  if (obj != registry.end()) return false;
  registry.insert(std::pair<std::string, material*>(instance->id, instance));
  return true;
}

void material_instances::remove(const std::string& id)
{
  auto obj = registry.find(id);
  if (obj != registry.end())
  {
    delete obj->second;
  }
  registry.erase(id);
}

material* material_instances::get_material(const std::string& id) const
{
  auto obj = registry.find(id);
  if (obj != registry.end())
  {
    return obj->second;
  }
  return nullptr;
}

std::vector<std::string> material_instances::get_material_ids() const
{
  std::vector<std::string> names;
  for (auto& pair : registry)
  {
    names.push_back(pair.first);
  }
  return names;
}

std::vector<std::string> material_instances::get_material_names() const
{
  std::vector<std::string> names;
  for (auto& pair : registry)
  {
    std::string name;
    pair.second->get_name(name, false);
    names.push_back(name);
  }
  return names;
}

int material_instances::get_index_by_name(const std::string& name) const
{
  std::vector<std::string> names = get_material_names();
  for (int i = 0; i < names.size();i++)
  {
    if (names[i] == name)
    {
      return i;
    }
  }
  return -1;
}

int material_instances::get_index_by_id(const std::string& id) const
{
  std::vector<std::string> ids = get_material_ids();
  for (int i = 0; i < ids.size(); i++)
  {
    if (ids[i] == id)
    {
      return i;
    }
  }
  return -1;
}

void material::get_name(std::string& out_name, bool with_params) const
{
  std::ostringstream oss;
  oss << "/" << material_class_names[(int)type] << "/" << id;
  out_name = oss.str();
}

void universal_material::get_name(std::string& out_name, bool with_params) const
{
  std::string base_name;
  material::get_name(base_name);
  if (with_params)
  {
    std::ostringstream oss;
    oss << base_name << "/" << color;
    out_name = oss.str();
  }
  else
  {
    out_name = base_name;
  }
}

void material::draw_edit_panel()
{
  std::string mat_name;
  get_name(mat_name);
  ImGui::Text("Material: ");
  ImGui::SameLine();
  ImGui::Text(mat_name.c_str());
}

void universal_material::draw_edit_panel()
{
  material::draw_edit_panel();
  ImGui::ColorEdit3("Color", color.e, ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSidePreview);
  ImGui::ColorEdit3("Emitted color", emitted_color.e, ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSidePreview);
  ImGui::InputFloat("Smoothness", &smoothness, 1);
  ImGui::Checkbox("Gloss enabled", &gloss_enabled);
  ImGui::InputFloat("Gloss probability", &smoothness, 1);
  ImGui::ColorEdit3("Gloss color", color.e, ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSidePreview);
}