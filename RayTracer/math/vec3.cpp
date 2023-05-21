#include "stdafx.h"

#include "common.h"

uint32_t vec3::get_type_hash() const
{
  return hash::combine(hash::get(x), hash::get(y), hash::get(z), hash::get(padding));
}