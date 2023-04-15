#include "stdafx.h"

#include "factories.h"
#include "math/materials.h"
#include "processing/async_renderer_base.h"
#include "renderers/example_renderer.h"
#include "renderers/reference_renderer.h"

material* object_factory::spawn_material(material_class type)
{
  if (type == material_class::universal) { return new material(material_class::universal); }
  else if (type == material_class::light) { return new material(material_class::light); }
  return nullptr;
}

async_renderer_base* object_factory::spawn_renderer(renderer_type type)
{
  if (type == renderer_type::example) { return new example_renderer(); }
  else if (type == renderer_type::reference) { return new reference_renderer(); }
  return nullptr;
}