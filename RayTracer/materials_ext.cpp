#include "stdafx.h"

#include <iosfwd>

#include "materials.h"
#include "imgui.h"


void materials_collection::add(material* instance)
{
  instances.push_back(instance);
}

void materials_collection::remove(int instance_id)
{
  delete instances[instance_id];
  instances.erase(instances.begin() + instance_id);
}

material* materials_collection::get_material(int index)
{
  if (index >= 0 && index < instances.size())
  {
    return instances[index];
  }
  return nullptr;
}

std::vector<std::string> materials_collection::get_material_names()
{
  std::vector<std::string> names;
  for (material* instance : instances)
  {
    std::string name;
    instance->get_name(name);
    names.push_back(name);
  }
  return names;
}


void material::get_name(std::string& out_name) const
{
  std::ostringstream oss;
  oss << material_type_names[(int)type] << "; " << friendly_name;
  out_name = oss.str();
}

void diffuse_material::get_name(std::string& out_name) const
{
  std::string base_name;
  material::get_name(base_name);
  std::ostringstream oss;
  oss << base_name << "; " << albedo.x << " " << albedo.x << " " << albedo.x;
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
  oss << base_name << "; " << albedo.x << " " << albedo.x << " " << albedo.x << "; " << fuzz;
  out_name = oss.str();
}

void dialectric_material::get_name(std::string& out_name) const
{
  std::string base_name;
  material::get_name(base_name);
  std::ostringstream oss;
  oss << base_name << "; " << index_of_refraction;
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

  char* x = &*friendly_name.begin();
  ImGui::InputText("Friendly name", x, 100);
  friendly_name = x;
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