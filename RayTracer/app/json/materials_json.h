#pragma once

#include "math/materials.h"

#include "app/json/serializable.h"

class material_serializer
{
public:
  static nlohmann::json serialize(const material& value);
  static material deserialize(const nlohmann::json& j);
};

class material_instances_serializer
{
public:
  static nlohmann::json serialize(const material_instances& value);
  static material_instances deserialize(const nlohmann::json& j);
};
