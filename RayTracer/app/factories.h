#pragma once

// No RTTI, simple type detection for each object type

enum class material_type
{
  none = 0,
  universal,
  light
};
static inline const char* material_type_names[] =
{
  "None",
  "Universal",
  "Light"
};


enum class renderer_type
{
  example = 0,
  preview,
  reference,
  ispc
};
static inline const char* renderer_type_names[] =
{
  "CPU Example",
  "CPU Preview",
  "CPU Reference",
  "CPU ISPC (Example only)"
};


class object_factory
{
public:
  static class material* spawn_material(material_type type);

  static class async_renderer_base* spawn_renderer(renderer_type type);
};