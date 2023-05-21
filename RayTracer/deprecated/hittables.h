#pragma once

#include <vector>
#include <array>
#include <string>

#include "ray.h"
#include "aabb.h"

#include "app/json/serializable.h"

class material;
class material_instances;

constexpr int32_t MAX_LIGHTS = 50;

struct hit_record
{
  vec3 p;         // hit point
  vec3 normal;
  float t;        // distance to hit point
  float u;
  float v;
  material* material_ptr = nullptr;
  bool front_face;
};

enum class hittable_class  // No RTTI, simple type detection
{
  scene=0,
  sphere,
  xy_rect,
  xz_rect,
  yz_rect
};
static inline const char* hittable_class_names[] =
{
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

  virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const = 0;
  virtual bool get_bounding_box(aabb& out_box) const = 0;
  virtual void get_name(std::string& out_name, bool with_params = true) const;
  virtual vec3 get_origin() const = 0;
  virtual vec3 get_extent() const = 0;
  virtual vec3 get_random_point() const = 0;
  virtual float get_area() const = 0;
  virtual void set_origin(const vec3& value) = 0;
  virtual void set_extent(float value) = 0;
  virtual float get_pdf_value(const vec3& origin, const vec3& v) const = 0;
  virtual vec3 get_pdf_direction(const vec3& look_from) const = 0;

  virtual void draw_edit_panel();
  virtual nlohmann::json serialize();
  virtual void deserialize(const nlohmann::json& j);
  virtual uint32_t get_type_hash() const;
  virtual hittable* clone() const = 0;

  // Persistent members
  hittable_class type = hittable_class::scene;
  std::string material_id;

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(hittable, type, material_id); // to_json only
  
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
  virtual vec3 get_origin() const override { return origin; };
  virtual vec3 get_extent() const override { return radius; };
  virtual float get_area() const override;
  virtual float get_pdf_value(const vec3& origin, const vec3& v) const override;
  virtual vec3 get_pdf_direction(const vec3& look_from) const override;
  virtual vec3 get_random_point() const override;
  virtual void set_origin(const vec3& value) override { origin = value; };
  virtual void set_extent(float value) override { radius = value; };

  virtual void draw_edit_panel() override;
  virtual nlohmann::json serialize() override;
  virtual void deserialize(const nlohmann::json& j) override;
  virtual uint32_t get_type_hash() const override;
  virtual sphere* clone() const override;

  // Persistent members
  vec3 origin = { 0,0,0 };
  float radius = 0.0f;

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(sphere, radius); // to_json only
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
  virtual vec3 get_origin() const override { assert(false); return vec3(0, 0, 0); };
  virtual vec3 get_extent() const override { assert(false); return vec3(0, 0, 0); };
  virtual float get_area() const override { assert(false); return 0.0f; };
  virtual float get_pdf_value(const vec3& origin, const vec3& v) const override { assert(false); return 0.0f; };
  virtual vec3 get_pdf_direction(const vec3& look_from) const override { assert(false); return vec3(0, 0, 0); };
  virtual vec3 get_random_point() const override { assert(false); return vec3(0, 0, 0); };
  virtual void set_origin(const vec3& value) override { assert(false); };
  virtual void set_extent(float value) { assert(false); };

  virtual void draw_edit_panel() override;
  virtual nlohmann::json serialize() override;
  virtual void deserialize(const nlohmann::json& j) override;
  virtual uint32_t get_type_hash() const override;
  virtual scene* clone() const override;

  void add(hittable* object);
  void remove(int object_id);

  void build_boxes();
  void update_materials(material_instances* materials);
  void override_texture_material(material* texture);
  void query_lights();
  hittable* get_random_light();

  std::vector<hittable*> objects;
  int32_t lights_num = 0;
  std::array<hittable*, MAX_LIGHTS> lights;
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
  virtual vec3 get_origin() const override { return vec3(x0, y0, z); };
  virtual vec3 get_extent() const override { return vec3(x1-x0, y1-y0, 0.0f); };
  virtual float get_area() const override;
  virtual float get_pdf_value(const vec3& origin, const vec3& v) const override;
  virtual vec3 get_pdf_direction(const vec3& look_from) const override;
  virtual vec3 get_random_point() const override;
  virtual void set_origin(const vec3& value) override { x0 = value.x; y0 = value.y; };
  virtual void set_extent(float value) { x1 = x0 + value; y1 = y0 + value; };

  virtual void draw_edit_panel() override;
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

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(xy_rect, x0, y0, x1, y1, z); // to_json only
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
  virtual vec3 get_origin() const override { return vec3(x0, y, z0); };
  virtual vec3 get_extent() const override { return vec3(x1-x0, 0.0f, z1-z0); };
  virtual float get_area() const override;
  virtual float get_pdf_value(const vec3& origin, const vec3& v) const override;
  virtual vec3 get_pdf_direction(const vec3& look_from) const override;
  virtual vec3 get_random_point() const override;
  virtual void set_origin(const vec3& value) override { x0 = value.x; z0 = value.z; };
  virtual void set_extent(float value) { x1 = x0 + value; z1 = z0 + value; };

  virtual void draw_edit_panel() override;
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

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(xz_rect, x0, z0, x1, z1, y); // to_json only
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
  virtual vec3 get_origin() const override { return vec3(x, y0, z0); };
  virtual vec3 get_extent() const override { return vec3(0.0f, y1-y0, z1-z0); };
  virtual float get_area() const override;
  virtual float get_pdf_value(const vec3& origin, const vec3& v) const override;
  virtual vec3 get_pdf_direction(const vec3& look_from) const override;
  virtual vec3 get_random_point() const override;
  virtual void set_origin(const vec3& value) override { y0 = value.y; z0 = value.z; };
  virtual void set_extent(float value) { y1 = y0 + value; z1 = z0 + value; };

  virtual void draw_edit_panel() override;
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

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(yz_rect, y0, z0, y1, z1, x); // to_json only
};
