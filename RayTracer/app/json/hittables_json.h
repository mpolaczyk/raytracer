#pragma once

#include "app/json/serializable.h"

class hittable;
class sphere;
class scene;
class xy_rect;
class xz_rect;
class yz_rect;
class static_mesh;

class hittable_serializer
{
public:
  static nlohmann::json serialize(const hittable* value);
  static void deserialize(const nlohmann::json& j, hittable* out_value);
};

class sphere_serializer
{
public:
  static nlohmann::json serialize(const sphere* value);
  static void deserialize(const nlohmann::json& j, sphere* out_value);
};

class scene_serializer
{
public:
  static nlohmann::json serialize(const scene* value);
  static void deserialize(const nlohmann::json& j, scene* out_value);
};

class xy_rect_serializer
{
public:
  static nlohmann::json serialize(const xy_rect* value);
  static void deserialize(const nlohmann::json& j, xy_rect* out_value);
};

class xz_rect_serializer
{
public:
  static nlohmann::json serialize(const xz_rect* value);
  static void deserialize(const nlohmann::json& j, xz_rect* out_value);
};

class yz_rect_serializer
{
public:
  static nlohmann::json serialize(const yz_rect* value);
  static void deserialize(const nlohmann::json& j, yz_rect* out_value);
};

class static_mesh_serializer
{
public:
  static nlohmann::json serialize(const static_mesh* value);
  static void deserialize(const nlohmann::json& j, static_mesh* out_value);
};