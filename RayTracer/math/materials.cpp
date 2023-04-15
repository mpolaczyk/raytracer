#include "stdafx.h"

#include "materials.h"
#include "common.h"
#include "pdf.h"

material* material::spawn_by_type(material_class type)
{
  if (type == material_class::universal) { return new material(material_class::universal); }
  else if (type == material_class::light) { return new material(material_class::light); }
  return nullptr;
}
