#pragma once

#include "ray.h"
#include "aabb.h"

#include "app/factories.h"

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
  class hittable* object = nullptr;
};

/* Warning! Adding new hittables:
* 1. Add child class in this header.
* 2. Add serializer and deserializer in hittables_json.h and hittables_json.cpp so that the type can be persistent.
* 3. Extend scene_serializer::serialize and scene_serializer::deserialize so that scene knows how to serialize sub objects.
* 4. Add draw_edit_panel and get_name in hittables_ui.cpp so that editor knows how to display UI elements for it.
* 5. Add get_hash in hittables.cpp so that program knows when object changed.
* 6. Add clone in hittable.cpp so that object can be cloned.
*/

class hittable
{
public:
  hittable() {}
  explicit hittable(const hittable* rhs) { *this = *rhs; };
  hittable(std::string&& in_material_id, hittable_type in_type) : material_id(std::move(in_material_id)), type(in_type) { };

  virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const = 0;
  virtual bool get_bounding_box(aabb& out_box) const = 0;
  virtual void get_name(std::string& out_name, bool with_params = true) const;
  virtual vec3 get_origin() const = 0;
  virtual vec3 get_extent() const = 0;
  virtual void set_origin(const vec3& value) = 0;
  virtual void set_extent(float value) = 0;
  // Deprecated begin
  virtual float get_area() const { assert(false); return 0.0f; };
  virtual float get_pdf_value(const vec3& origin, const vec3& v) const { assert(false); return 0.0f; };
  virtual vec3 get_pdf_direction(const vec3& look_from) const { assert(false); return vec3(0, 0, 0); };
  virtual vec3 get_random_point() const { assert(false); return vec3(0, 0, 0); };
  // Deprecated end

  virtual void draw_edit_panel();
  virtual uint32_t get_hash() const;
  virtual hittable* clone() const = 0;
  virtual void load_resources() {};

  // Persistent members
  hittable_type type = hittable_type::scene;
  std::string material_id;
    
  // Runtime members
  material* material_ptr = nullptr; // no deep copy for now!
  aabb bounding_box;
};


class sphere : public hittable
{
public:
  sphere() : hittable("", hittable_type::sphere) {}
  explicit sphere(const sphere* rhs) { *this = *rhs; };
  sphere(std::string&& in_material_id, const vec3& in_origin, float radius) : origin(in_origin), radius(radius), hittable(std::move(in_material_id), hittable_type::sphere) { };

  virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const override;
  virtual bool get_bounding_box(aabb& out_box) const override;
  virtual void get_name(std::string& out_name, bool with_params) const override;
  virtual vec3 get_origin() const override { return origin; };
  virtual vec3 get_extent() const override { return vec3(radius); };
  virtual void set_origin(const vec3& value) override { origin = value; };
  virtual void set_extent(float value) override { radius = value; };
  // Deprecated begin
  virtual float get_area() const override;
  virtual float get_pdf_value(const vec3& origin, const vec3& v) const override;
  virtual vec3 get_pdf_direction(const vec3& look_from) const override;
  virtual vec3 get_random_point() const override;
  // Deprecated end

  virtual void draw_edit_panel() override;
  virtual uint32_t get_hash() const override;
  virtual sphere* clone() const override;

  // Persistent members
  vec3 origin = { 0,0,0 };
  float radius = 0.0f;
};


class scene : public hittable
{
public:
  scene() : hittable("", hittable_type::scene) {}
  explicit scene(const scene* rhs) { *this = *rhs; };
  ~scene();

  virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const override;
  virtual bool get_bounding_box(aabb& out_box) const override;
  virtual void get_name(std::string& out_name, bool with_params) const override;
  virtual vec3 get_origin() const override { assert(false); return vec3(0, 0, 0); };
  virtual vec3 get_extent() const override { assert(false); return vec3(0, 0, 0); };
  virtual void set_origin(const vec3& value) override { };
  virtual void set_extent(float value) override { assert(false); };

  virtual void draw_edit_panel() override;
  virtual uint32_t get_hash() const override;
  virtual scene* clone() const override;
  virtual void load_resources() override;

  void add(hittable* object);
  void remove(int object_id);

  void build_boxes();
  void update_materials(material_instances* materials);
  void query_lights();
  hittable* get_random_light();

  // Persistent members
  std::vector<hittable*> objects;

  // Runtime members
  int32_t lights_num = 0;
  std::array<hittable*, MAX_LIGHTS> lights;
};


class xy_rect : public hittable 
{
public:
  xy_rect() : hittable("", hittable_type::xy_rect) {}
  explicit xy_rect(const xy_rect* rhs) { *this = *rhs; };
  xy_rect(std::string&& in_material_id, float _x0, float _x1, float _y0, float _y1, float _z)
    : x0(_x0), x1(_x1), y0(_y0), y1(_y1), z(_z), hittable(std::move(in_material_id), hittable_type::xy_rect) { };

  virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const override;
  virtual bool get_bounding_box(aabb& out_box) const override;
  virtual void get_name(std::string& out_name, bool with_params) const override;
  virtual vec3 get_origin() const override { return vec3(x0, y0, z); };
  virtual vec3 get_extent() const override { return vec3(x1-x0, y1-y0, 0.0f); };
  virtual void set_origin(const vec3& value) override { x0 = value.x; y0 = value.y; };
  virtual void set_extent(float value) override { x1 = x0 + value; y1 = y0 + value; };
  // Deprecated begin
  virtual float get_area() const override;
  virtual float get_pdf_value(const vec3& origin, const vec3& v) const override;
  virtual vec3 get_pdf_direction(const vec3& look_from) const override;
  virtual vec3 get_random_point() const override;
  // Deprecated end

  virtual void draw_edit_panel() override;
  virtual uint32_t get_hash() const override;
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
};

class xz_rect : public hittable
{
public:
  xz_rect() : hittable("", hittable_type::xz_rect) {}
  explicit xz_rect(const xz_rect* rhs) { *this = *rhs; };
  xz_rect(std::string&& in_material_id, float _x0, float _x1, float _z0, float _z1, float _y)
    : x0(_x0), x1(_x1), z0(_z0), z1(_z1), y(_y), hittable(std::move(in_material_id), hittable_type::xz_rect) { };

  virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const override;
  virtual bool get_bounding_box(aabb& out_box) const override;
  virtual void get_name(std::string& out_name, bool with_params) const override;
  virtual vec3 get_origin() const override { return vec3(x0, y, z0); };
  virtual vec3 get_extent() const override { return vec3(x1-x0, 0.0f, z1-z0); };
  virtual void set_origin(const vec3& value) override { x0 = value.x; z0 = value.z; };
  virtual void set_extent(float value) override { x1 = x0 + value; z1 = z0 + value; };
  // Deprecated begin
  virtual float get_area() const override;
  virtual float get_pdf_value(const vec3& origin, const vec3& v) const override;
  virtual vec3 get_pdf_direction(const vec3& look_from) const override;
  virtual vec3 get_random_point() const override;
  // Deprecated end

  virtual void draw_edit_panel() override;
  virtual uint32_t get_hash() const override;
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
};

class yz_rect : public hittable 
{
public:
  yz_rect() : hittable("", hittable_type::yz_rect) {}
  explicit yz_rect(const yz_rect* rhs) { *this = *rhs; };
  yz_rect(std::string&& in_material_id, float _y0, float _y1, float _z0, float _z1, float _x)
    : y0(_y0), y1(_y1), z0(_z0), z1(_z1), x(_x), hittable(std::move(in_material_id), hittable_type::yz_rect) { };

  virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const override;
  virtual bool get_bounding_box(aabb& out_box) const override;
  virtual void get_name(std::string& out_name, bool with_params) const override;
  virtual vec3 get_origin() const override { return vec3(x, y0, z0); };
  virtual vec3 get_extent() const override { return vec3(0.0f, y1-y0, z1-z0); };
  virtual void set_origin(const vec3& value) override { y0 = value.y; z0 = value.z; };
  virtual void set_extent(float value) override { y1 = y0 + value; z1 = z0 + value; };
  // Deprecated begin
  virtual float get_area() const override;
  virtual float get_pdf_value(const vec3& origin, const vec3& v) const override;
  virtual vec3 get_pdf_direction(const vec3& look_from) const override;
  virtual vec3 get_random_point() const override;
  // Deprecated end

  virtual void draw_edit_panel() override;
  virtual uint32_t get_hash() const override;
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
};

class static_mesh : public hittable
{
public:
  static_mesh() : hittable("", hittable_type::static_mesh) {}
  explicit static_mesh(const static_mesh* rhs) { *this = *rhs; };

  virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const override;
  virtual bool get_bounding_box(aabb& out_box) const override;
  virtual void get_name(std::string& out_name, bool with_params) const override;
  virtual vec3 get_origin() const override { return origin; };
  virtual vec3 get_extent() const override { return vec3(extent); };
  virtual void set_origin(const vec3& value) override { origin = value; };
  virtual void set_extent(float value) override { extent = value; };

  virtual void draw_edit_panel() override;
  virtual uint32_t get_hash() const override;
  virtual static_mesh* clone() const override;
  virtual void load_resources() override;
  
  // Persistent state
  std::string file_name;
  vec3 origin = { 0,0,0 };
  vec3 scale = { 1,1,1 };
  vec3 rotation = { 0,0,0 };
  int32_t shape_index = 0;

  // Runtime state
  std::vector<obj_helper::triangle_face> faces;
  float extent = 0.0f;
  bool resources_dirty = true;
};