#pragma once

#include "app/app.h"
#include "math/camera.h"

#include "app/json/serializable.h"

class window_config_serializer
{
public:
  static nlohmann::json serialize(const window_config& value);
  static window_config deserialize(const nlohmann::json& j);
};

class camera_config_serializer
{
public:
  static nlohmann::json serialize(const camera_config& value);
  static camera_config deserialize(const nlohmann::json& j);
};