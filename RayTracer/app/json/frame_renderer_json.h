#pragma once

#include "processing/async_renderer_base.h"

#include "app/json/serializable.h"

class renderer_config_serializer
{
public:
  static nlohmann::json serialize(const renderer_config& value);
  static renderer_config deserialize(const nlohmann::json& j);
};
