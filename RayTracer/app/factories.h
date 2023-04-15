#pragma once

enum class material_class  // No RTTI, simple type detection
{
  none = 0,
  universal,
  light
};
static inline const char* material_class_names[] =
{
  "None",
  "Universal",
  "Light"
};


enum class renderer_type
{
  example = 0,
  reference
};
static inline const char* renderer_names[] =
{
  "Example",
  "Reference"
};

class object_factory
{
public:
  static class material* spawn_material(material_class type);

  static class async_renderer_base* spawn_renderer(renderer_type type);
};