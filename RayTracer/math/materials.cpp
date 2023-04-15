#include "stdafx.h"

#include "materials.h"
#include "common.h"
#include "pdf.h"

material* material::spawn_by_type(material_class type)
{
  if (type == material_class::universal) { return new universal_material(); }
  return nullptr;
}

vec3 material::get_color() const
{
  return c_black;
}

vec3 material::get_emitted() const
{
  return c_white;
}

float material::get_smoothness() const
{
  return 0.5f;  // More smoothness -> more shiny, more specular
}

bool material::get_gloss_enabled() const
{
  return false;
}

float material::get_gloss_probability() const
{
  return 0.2;
}
vec3 material::get_gloss_color() const
{
  return c_white;
}