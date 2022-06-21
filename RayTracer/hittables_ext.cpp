#include "stdafx.h"

#include "hittables.h"
#include "materials.h"

#include <iosfwd>

#include "imgui.h"

void hittable::get_name(std::string& out_name) const
{
  std::string base_name = hittable_type_names[(int)type];
  
  if (mat != nullptr)
  {
    std::ostringstream oss;
    oss << base_name << "; " << mat->friendly_name;
    out_name = oss.str();
  }
  else
  {
    out_name = base_name;
  }
  
}

void hittable_list::get_name(std::string& out_name) const
{
  out_name = hittable_type_names[(int)hittable_type::hittable_list];
}

void sphere::get_name(std::string& out_name) const
{
  std::string base_name;
  hittable::get_name(base_name);
  std::ostringstream oss;
  oss << base_name << "; " << radius;
  out_name = oss.str();
}

void xy_rect::get_name(std::string& out_name) const
{
  std::string base_name;
  hittable::get_name(base_name);
  std::ostringstream oss;
  oss << base_name << "; " << x0 << " " << y0 << "; " << x1 << " " << y1;
  out_name = oss.str();
}

void xz_rect::get_name(std::string& out_name) const
{
  std::string base_name;
  hittable::get_name(base_name);
  std::ostringstream oss;
  oss << base_name << "; " << x0 << " " << z0 << "; " << x1 << " " << z1;
  out_name = oss.str();
}

void yz_rect::get_name(std::string& out_name) const
{
  std::string base_name;
  hittable::get_name(base_name);
  std::ostringstream oss;
  oss << base_name << "; " << y0 << " " << z0 << "; " << y1 << " " << z1;
  out_name = oss.str();
}


void hittable::draw_edit_panel()
{
  std::string hittable_name;
  get_name(hittable_name);
  ImGui::Text("Object: ");
  ImGui::SameLine();
  ImGui::Text(hittable_name.c_str());
}

void hittable_list::draw_edit_panel()
{
  hittable::draw_edit_panel();
}

void sphere::draw_edit_panel()
{
  hittable::draw_edit_panel();
  ImGui::InputFloat3("Origin", origin.e, "%f.2");
  ImGui::InputFloat("Radius", &radius, 1);
  if (mat != nullptr)
  {
    ImGui::Separator();
    mat->draw_edit_panel();
  }
}

void xy_rect::draw_edit_panel()
{
  hittable::draw_edit_panel();
  ImGui::InputFloat2("x0 y0", x0y0);
  ImGui::InputFloat2("x1 y1", x1y1);
  ImGui::InputFloat("z", &z);
  if (mat != nullptr)
  {
    ImGui::Separator();
    mat->draw_edit_panel();
  }
}

void xz_rect::draw_edit_panel()
{
  hittable::draw_edit_panel();
  ImGui::InputFloat2("x0 z0", x0z0);
  ImGui::InputFloat2("x1 z1", x1z1);
  ImGui::InputFloat("y", &y);
  if (mat != nullptr)
  {
    ImGui::Separator();
    mat->draw_edit_panel();
  }
}

void yz_rect::draw_edit_panel()
{
  hittable::draw_edit_panel();
  ImGui::InputFloat2("y0 z0", y0z0);
  ImGui::InputFloat2("y1 z1", y1z1);
  ImGui::InputFloat("x", &x);
  if (mat != nullptr)
  {
    ImGui::Separator();
    mat->draw_edit_panel();
  }
}