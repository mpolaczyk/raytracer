#include "stdafx.h"

#include "common.h"

uint32_t vec3::get_type_hash() const
{
  return hash_combine(::get_type_hash(x), ::get_type_hash(y), ::get_type_hash(z), ::get_type_hash(padding));
}