#include "stdafx.h"

#include "material.h"

#include <iosfwd>

#include "imgui.h"

void material::get_name(std::string& out_name) const
{
  out_name = material_type_names[(int)type];
}

void diffuse_material::get_name(std::string& out_name) const
{
  std::string base_name = material_type_names[(int)type];
  std::ostringstream oss;
  oss << base_name << " " << albedo.x << " " << albedo.x << " " << albedo.x;
  out_name = oss.str();
}

void texture_material::get_name(std::string& out_name) const
{
  out_name = material_type_names[(int)type];
}

void metal_material::get_name(std::string& out_name) const
{
  std::string base_name = material_type_names[(int)type];
  std::ostringstream oss;
  oss << base_name << " " << albedo.x << " " << albedo.x << " " << albedo.x << " " << fuzz;
  out_name = oss.str();
}

void dialectric_material::get_name(std::string& out_name) const
{
  std::string base_name = material_type_names[(int)type];
  std::ostringstream oss;
  oss << base_name << " " << index_of_refraction;
  out_name = oss.str();
}

void diffuse_light_material::get_name(std::string& out_name) const
{
  out_name = material_type_names[(int)type];
}



void material::draw_edit_panel()
{
  ImGui::Text("Nothing to edit");
}

void diffuse_material::draw_edit_panel()
{
  ImGui::ColorEdit3("Albedo", albedo.e, ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSidePreview);
  
}

void texture_material::draw_edit_panel()
{
  ImGui::Text("TODO add texture input");
}

void metal_material::draw_edit_panel()
{
  ImGui::ColorEdit3("Albedo", albedo.e, ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSidePreview);
  ImGui::InputFloat("Fuzz", &fuzz, 1);
}

void dialectric_material::draw_edit_panel()
{
  ImGui::InputFloat("Index of refraction", &index_of_refraction, 1);
}

void diffuse_light_material::draw_edit_panel()
{
  ImGui::Text("TODO add texture input");
}