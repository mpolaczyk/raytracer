#include "stdafx.h"

#include "factories.h"

#include "math/materials.h"

#include "processing/async_renderer_base.h"
#include "renderers/example_renderer.h"
#include "renderers/preview_renderer.h"
#include "renderers/preview_normals_renderer.h"
#include "renderers/preview_faces_renderer.h"
#include "renderers/reference_renderer.h"
#include "renderers/ispc_renderer.h"

#include "math/hittables.h"

material* object_factory::spawn_material(material_type type)
{
  if (type == material_type::universal) { return new material(material_type::universal); }
  else if (type == material_type::light) { return new material(material_type::light); }
  return nullptr;
}

async_renderer_base* object_factory::spawn_renderer(renderer_type type)
{
  if (type == renderer_type::example) { return new example_renderer(); }
  else if (type == renderer_type::preview) { return new preview_renderer(); }
  else if (type == renderer_type::preview_normals) { return new preview_normals_renderer(); }
  else if (type == renderer_type::preview_faces) { return new preview_faces_renderer(); }
  else if (type == renderer_type::reference) { return new reference_renderer(); }
  else if (type == renderer_type::ispc) { return new ispc_renderer(); }
  return nullptr;
}

hittable* object_factory::spawn_hittable(hittable_type type)
{
  hittable* obj = nullptr;
  if (type == hittable_type::scene) { obj = new scene(); }
  else if (type == hittable_type::sphere) { obj = new sphere(); }
  else if (type == hittable_type::xy_rect) { obj = new xy_rect(); }
  else if (type == hittable_type::xz_rect) { obj = new xz_rect(); }
  else if (type == hittable_type::yz_rect) { obj = new yz_rect(); }
  else if (type == hittable_type::static_mesh) { obj = new static_mesh(); }

  if (obj != nullptr)
  {
    obj->id = hittable::last_id;
    hittable::last_id++;
  }
  return obj;
}