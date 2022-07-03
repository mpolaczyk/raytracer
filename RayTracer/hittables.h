#pragma once

#include <vector>
#include <string>

#include "ray.h"
#include "aabb.h"

#include "nlohmann\json.hpp"
#include "serializable.h"

class material;
class material_instances;

struct hit_record
{
  vec3 p;
  vec3 normal;
  float t;
  float u;
  float v;
  material* material_ptr = nullptr;
  bool front_face;
};

enum class hittable_class  // No RTTI, simple type detection
{
  hittable=0,
  scene,
  sphere,
  xy_rect,
  xz_rect,
  yz_rect
};
static inline const char* hittable_class_names[] =
{
  "Hittable",
  "Scene",
  "Sphere",
  "XY Rectangle",
  "XZ Rectangle",
  "YZ Rectangle"
};

class hittable : serializable<nlohmann::json>
{
public:
  hittable() {}
  hittable(const hittable* rhs) { *this = *rhs; };
  hittable(std::string&& in_material_id, hittable_class in_type) : material_id(std::move(in_material_id)), type(in_type) { };

  virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const { return false; };
  virtual bool get_bounding_box(aabb& out_box) const { return false; };
  virtual void get_name(std::string& out_name, bool with_params = true) const;
  virtual void draw_edit_panel();
  virtual void set_origin(const vec3& value) {};
  virtual void set_extent(float value) {};
  virtual nlohmann::json serialize();
  virtual void deserialize(const nlohmann::json& j);

  virtual uint32_t get_type_hash() const;
  virtual hittable* clone() const;

  // Persistent members
  hittable_class type = hittable_class::hittable;
  std::string material_id;

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(hittable, type, material_id);
  
  // Runtime members
  material* material_ptr = nullptr; // no deep copy for now!
  aabb bounding_box;

  static hittable* spawn_by_type(hittable_class type);
};


class sphere : public hittable
{
public:
  sphere() : hittable("", hittable_class::sphere) {}
  sphere(const sphere* rhs) { *this = *rhs; };
  sphere(std::string&& in_material_id, vec3 in_origin, float radius) : origin(in_origin), radius(radius), hittable(std::move(in_material_id), hittable_class::sphere) { };

  virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const override;
  virtual bool get_bounding_box(aabb& out_box) const override;
  virtual void get_name(std::string& out_name, bool with_params) const override;
  virtual void draw_edit_panel() override;
  virtual void set_origin(const vec3& value) override { origin = value; };
  virtual void set_extent(float value) { radius = value; };
  virtual nlohmann::json serialize() override;
  virtual void deserialize(const nlohmann::json& j) override;

  virtual uint32_t get_type_hash() const override;
  virtual sphere* clone() const override;

  // Persistent members
  vec3 origin = { 0,0,0 };
  float radius = 0.0f;

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(sphere, radius);
};


class scene : public hittable
{
public:
  scene() : hittable("", hittable_class::scene) {}
  scene(const scene* rhs) { *this = *rhs; };
  ~scene();

  virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const override;
  virtual bool get_bounding_box(aabb& out_box) const override;
  virtual void get_name(std::string& out_name, bool with_params) const override;
  virtual void draw_edit_panel() override;
  virtual void set_origin(const vec3& value) override { };
  virtual void set_extent(float value) { };
  virtual nlohmann::json serialize() override;
  virtual void deserialize(const nlohmann::json& j) override;

  virtual uint32_t get_type_hash() const override;
  virtual scene* clone() const override;

  void add(hittable* object) { objects.push_back(object); }
  void remove(int object_id) { delete objects[object_id]; objects.erase(objects.begin() + object_id); }
  void build_boxes();
  void update_materials(material_instances* materials);

  std::vector<hittable*> objects;
};


class xy_rect : public hittable 
{
public:
  xy_rect() : hittable("", hittable_class::xy_rect) {}
  xy_rect(const xy_rect* rhs) { *this = *rhs; };
  xy_rect(std::string&& in_material_id, float _x0, float _x1, float _y0, float _y1, float _z)
    : x0(_x0), x1(_x1), y0(_y0), y1(_y1), z(_z), hittable(std::move(in_material_id), hittable_class::xy_rect) { };

  virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const override;
  virtual bool get_bounding_box(aabb& out_box) const override;
  virtual void get_name(std::string& out_name, bool with_params) const override;
  virtual void draw_edit_panel() override;
  virtual void set_origin(const vec3& value) override { x0 = value.x; y0 = value.y; };
  virtual void set_extent(float value) { x1 = x0 + value; y1 = y0 + value; };
  virtual nlohmann::json serialize() override;
  virtual void deserialize(const nlohmann::json& j) override;

  virtual uint32_t get_type_hash() const override;
  virtual xy_rect* clone() const override;

public:
  union
  {
    float x0y0[2] = { 0.0f, 0.0f };
    struct { float x0, y0; };
  };
  union
  {
    float x1y1[2] = { 0.0f, 0.0f };
    struct { float x1, y1; };
  };
  float z = 0.0f;

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(xy_rect, x0, y0, x1, y1, z);
};

class xz_rect : public hittable
{
public:
  xz_rect() : hittable("", hittable_class::xz_rect) {}
  xz_rect(const xz_rect* rhs) { *this = *rhs; };
  xz_rect(std::string&& in_material_id, float _x0, float _x1, float _z0, float _z1, float _y)
    : x0(_x0), x1(_x1), z0(_z0), z1(_z1), y(_y), hittable(std::move(in_material_id), hittable_class::xz_rect) { };

  virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const override;
  virtual bool get_bounding_box(aabb& out_box) const override;
  virtual void get_name(std::string& out_name, bool with_params) const override;
  virtual void draw_edit_panel() override;
  virtual void set_origin(const vec3& value) override { x0 = value.x; z0 = value.z; };
  virtual void set_extent(float value) { x1 = x0 + value; z1 = z0 + value; };
  virtual nlohmann::json serialize() override;
  virtual void deserialize(const nlohmann::json& j) override;

  virtual uint32_t get_type_hash() const override;
  virtual xz_rect* clone() const override;

public:
  union
  {
    float x0z0[2] = { 0.0f, 0.0f };
    struct { float x0, z0; };
  };
  union
  {
    float x1z1[2] = { 0.0f, 0.0f };
    struct { float x1, z1; };
  };
  float y = 0.0f;

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(xz_rect, x0, z0, x1, z1, y);
};

class yz_rect : public hittable 
{
public:
  yz_rect() : hittable("", hittable_class::yz_rect) {}
  yz_rect(const yz_rect* rhs) { *this = *rhs; };
  yz_rect(std::string&& in_material_id, float _y0, float _y1, float _z0, float _z1, float _x)
    : y0(_y0), y1(_y1), z0(_z0), z1(_z1), x(_x), hittable(std::move(in_material_id), hittable_class::yz_rect) { };

  virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const override;
  virtual bool get_bounding_box(aabb& out_box) const override;
  virtual void get_name(std::string& out_name, bool with_params) const override;
  virtual void draw_edit_panel() override;
  virtual void set_origin(const vec3& value) override { y0 = value.y; z0 = value.z; };
  virtual void set_extent(float value) { y1 = y0 + value; z1 = z0 + value; };
  virtual nlohmann::json serialize() override;
  virtual void deserialize(const nlohmann::json& j) override;

  virtual uint32_t get_type_hash() const override;
  virtual yz_rect* clone() const override;

public:
  union
  {
    float y0z0[2] = { 0.0f, 0.0f };
    struct { float y0, z0; };
  };
  union
  {
    float y1z1[2] = { 0.0f, 0.0f };
    struct { float y1, z1; };
  };
  float x;

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(yz_rect, y0, z0, y1, z1, x);
};