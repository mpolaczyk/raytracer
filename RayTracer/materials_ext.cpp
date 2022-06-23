#include "stdafx.h"

#include <iosfwd>

#include "materials.h"
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
    pair.second->get_name(name);
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


void material::get_name(std::string& out_name) const
{
  std::ostringstream oss;
  oss << "/" << material_class_names[(int)type] << "/" << id;
  out_name = oss.str();
}

void diffuse_material::get_name(std::string& out_name) const
{
  std::string base_name;
  material::get_name(base_name);
  std::ostringstream oss;
  oss << base_name << "/(" << albedo.x << "," << albedo.y << "," << albedo.z << ")";
  out_name = oss.str();
}

void texture_material::get_name(std::string& out_name) const
{
  material::get_name(out_name);
}

void metal_material::get_name(std::string& out_name) const
{
  std::string base_name;
  material::get_name(base_name);
  std::ostringstream oss;
  oss << base_name << "/(" << albedo.x << "," << albedo.y << "," << albedo.z << ")," << fuzz;
  out_name = oss.str();
}

void dialectric_material::get_name(std::string& out_name) const
{
  std::string base_name;
  material::get_name(base_name);
  std::ostringstream oss;
  oss << base_name << "/" << index_of_refraction;
  out_name = oss.str();
}

void diffuse_light_material::get_name(std::string& out_name) const
{
  material::get_name(out_name);
}



void material::draw_edit_panel()
{
  std::string mat_name;
  get_name(mat_name);
  ImGui::Text("Material: ");
  ImGui::SameLine();
  ImGui::Text(mat_name.c_str());

  //char* x = &*id.begin();
  //ImGui::InputText("Friendly name", x, 100);
  //id = x;
}

void diffuse_material::draw_edit_panel()
{
  material::draw_edit_panel();
  ImGui::ColorEdit3("Albedo", albedo.e, ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSidePreview);
}

void texture_material::draw_edit_panel()
{
  material::draw_edit_panel();
  ImGui::Text("TODO add texture input");
}

void metal_material::draw_edit_panel()
{
  material::draw_edit_panel();
  ImGui::ColorEdit3("Albedo", albedo.e, ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSidePreview);
  ImGui::InputFloat("Fuzz", &fuzz, 1);
}

void dialectric_material::draw_edit_panel()
{
  material::draw_edit_panel();
  ImGui::InputFloat("Index of refraction", &index_of_refraction, 1);
}

void diffuse_light_material::draw_edit_panel()
{
  material::draw_edit_panel();
  ImGui::Text("TODO add texture input");
}