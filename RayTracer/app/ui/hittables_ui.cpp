#include "stdafx.h"

#include "imgui.h"

#include "math/hittables.h"
#include "math/materials.h"

void hittable::get_name(std::string& out_name, bool with_params) const
{
  std::string base_name = hittable_type_names[(int)type];
  
  std::ostringstream oss;
  oss << "[" << id << "]" << "/" << base_name << "/" << material_id;
  out_name = oss.str();
}

void scene::get_name(std::string& out_name, bool with_params) const
{
  out_name = hittable_type_names[(int)hittable_type::scene];
}

void sphere::get_name(std::string& out_name, bool with_params) const
{
  std::string base_name;
  hittable::get_name(base_name);
  if (with_params)
  {
    std::ostringstream oss;
    oss << base_name << "/" << radius;
    out_name = oss.str();
  }
  else
  {
    out_name = base_name;
  }
}

void xy_rect::get_name(std::string& out_name, bool with_params) const
{
  std::string base_name;
  hittable::get_name(base_name);
  if (with_params)
  {
    std::ostringstream oss;
    oss << base_name << "/(" << x0 << "," << y0 << "),(" << x1 << "," << y1 << ")," << z;
    out_name = oss.str();
  }
  else
  {
    out_name = base_name;
  }
}

void xz_rect::get_name(std::string& out_name, bool with_params) const
{
  std::string base_name;
  hittable::get_name(base_name);
  if (with_params)
  {
    std::ostringstream oss;
    oss << base_name << "/(" << x0 << "," << z0 << ")," << y << ",(" << x1 << "," << z1 << ")";
    out_name = oss.str();
  }
  else
  {
    out_name = base_name;
  }
}

void yz_rect::get_name(std::string& out_name, bool with_params) const
{
  std::string base_name;
  hittable::get_name(base_name);
  if (with_params)
  {
    std::ostringstream oss;
    oss << base_name << "/" << x << ",(" << y0 << "," << z0 << "),(" << y1 << "," << z1 << ")";
    out_name = oss.str();
  }
  else
  {
    out_name = base_name;
  }
}

void static_mesh::get_name(std::string& out_name, bool with_params) const
{
  std::string base_name;
  hittable::get_name(base_name);
  if (with_params)
  {
    std::ostringstream oss;
    oss << base_name << "/" << file_name;
    out_name = oss.str();
  }
  else
  {
    out_name = base_name;
  }
}

void hittable::draw_edit_panel()
{
  std::string hittable_name;
  get_name(hittable_name, false);
  ImGui::Text("Object: ");
  ImGui::SameLine();
  ImGui::Text(hittable_name.c_str());
}

void scene::draw_edit_panel()
{
  hittable::draw_edit_panel();
}

void sphere::draw_edit_panel()
{
  hittable::draw_edit_panel();
  ImGui::DragFloat3("Origin", origin.e);
  ImGui::DragFloat("Radius", &radius, 1);
}

void xy_rect::draw_edit_panel()
{
  hittable::draw_edit_panel();
  ImGui::DragFloat2("x0 y0", x0y0);
  ImGui::DragFloat2("x1 y1", x1y1);
  ImGui::DragFloat("z", &z);
}

void xz_rect::draw_edit_panel()
{
  hittable::draw_edit_panel();
  ImGui::DragFloat2("x0 z0", x0z0);
  ImGui::DragFloat2("x1 z1", x1z1);
  ImGui::DragFloat("y", &y);
}

void yz_rect::draw_edit_panel()
{
  hittable::draw_edit_panel();
  ImGui::DragFloat2("y0 z0", y0z0);
  ImGui::DragFloat2("y1 z1", y1z1);
  ImGui::DragFloat("x", &x);
}

void static_mesh::draw_edit_panel()
{
  hittable::draw_edit_panel();
  ImGui::DragFloat3("Origin", origin.e);
  ImGui::DragFloat3("Rotation", rotation.e);
  ImGui::DragFloat3("Scale", scale.e);
  {
    assert(file_name.size() <= 256);
    char* buffer = new char[256];
    strcpy(buffer, file_name.c_str());
    if (ImGui::InputText("Object file", buffer, 256))
    {
      file_name = buffer;
      resources_dirty = true;
    }
    delete[] buffer;
  }
  if (ImGui::InputInt("Shape index", &shape_index, 1))
  {
    resources_dirty = true;
  }
}