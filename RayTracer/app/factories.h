#pragma once

// No RTTI, simple type detection for each object type

class material;
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

class async_renderer_base;
enum class renderer_type
{
  example = 0,
  preview,
  preview_normals,
  preview_faces,
  reference,
  ispc,
  gpu_reference
};
static inline const char* renderer_type_names[] =
{
  "CPU Example",
  "CPU Preview",
  "CPU Preview Normals",
  "CPU Preview Faces",
  "CPU Reference",
  "CPU ISPC (example)",
  "GPU Reference"
};

class hittable;
enum class hittable_type
{
  scene = 0,
  sphere,
  xy_rect,
  xz_rect,
  yz_rect,
  static_mesh
};
static inline const char* hittable_type_names[] =
{
  "Scene",
  "Sphere",
  "XY Rectangle",
  "XZ Rectangle",
  "YZ Rectangle",
  "Static Mesh"
};

class object_factory
{
public:
  static material* spawn_material(material_type type);

  static async_renderer_base* spawn_renderer(renderer_type type);

  static hittable* spawn_hittable(hittable_type type);
};