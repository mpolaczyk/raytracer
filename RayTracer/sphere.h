#pragma once

#include <vector>

#include "ray.h"
#include "aabb.h"

class material;

struct hit_record
{
  vec3 p;
  vec3 normal;
  float t;
  float u;
  float v;
  material* mat;
  bool front_face;
};

enum class hittable_type  // No RTTI, simple type detection
{
  hittable=0,
  hittable_list,
  sphere,
  xy_rect,
  xz_rect,
  yz_rect
};

class hittable
{
public:
  hittable(material* in_mat, hittable_type in_type) : mat(in_mat), type(in_type) { };

  virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const = 0;
  virtual bool get_bounding_box(aabb& out_box) const = 0;

  material* mat = nullptr;
  aabb bounding_box;
  hittable_type type = hittable_type::hittable;
};


class sphere : public hittable
{
public:
  sphere(vec3 in_origin, float radius, material* in_mat) : origin(in_origin), radius(radius), hittable(in_mat, hittable_type::sphere) { };

  virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const override;
  virtual bool get_bounding_box(aabb& out_box) const override;

public:
  vec3 origin;
  float radius;
};


class hittable_list : public hittable
{
public:
  hittable_list() : hittable(nullptr, hittable_type::hittable_list) {}

  virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const override;
  virtual bool get_bounding_box(aabb& out_box) const override;

  void clear() { objects.clear(); }
  void add(hittable* object) { objects.push_back(object); }
  void build_boxes();

public:
  std::vector<hittable*> objects;
};


class xy_rect : public hittable 
{
public:
  xy_rect() : hittable(nullptr, hittable_type::xy_rect) {}

  xy_rect(float _x0, float _x1, float _y0, float _y1, float _k, material* mat)
    : x0(_x0), x1(_x1), y0(_y0), y1(_y1), k(_k), hittable(mat, hittable_type::xy_rect) { };

  virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const override;
  virtual bool get_bounding_box(aabb& out_box) const override;

public:
  float x0, x1, y0, y1, k;
};

class xz_rect : public hittable
{
public:
  xz_rect() : hittable(nullptr, hittable_type::xz_rect) {}

  xz_rect(float _x0, float _x1, float _z0, float _z1, float _k, material* mat)
    : x0(_x0), x1(_x1), z0(_z0), z1(_z1), k(_k), hittable(mat, hittable_type::xz_rect) { };

  virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const override;
  virtual bool get_bounding_box(aabb& out_box) const override;

public:
  float x0, x1, z0, z1, k;
};

class yz_rect : public hittable 
{
public:
  yz_rect() : hittable(nullptr, hittable_type::yz_rect) {}

  yz_rect(float _y0, float _y1, float _z0, float _z1, float _k, material* mat)
    : y0(_y0), y1(_y1), z0(_z0), z1(_z1), k(_k), hittable(mat, hittable_type::yz_rect) { };

  virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const override;
  virtual bool get_bounding_box(aabb& out_box) const override;

public:
  float y0, y1, z0, z1, k;
};