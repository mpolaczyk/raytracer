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

class hittable
{
public:
  hittable(material* in_mat) : mat(in_mat) { };

  virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const = 0;
  virtual bool get_bounding_box(aabb& out_box) const = 0;

  material* mat = nullptr;
  aabb bounding_box;
};

class sphere : public hittable
{
public:
  //sphere() = default;
  sphere(vec3 in_origin, float radius, material* in_mat) : origin(in_origin), radius(radius), hittable(in_mat) { };

  virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const override;
  virtual bool get_bounding_box(aabb& out_box) const override;

public:
  vec3 origin;
  float radius;
};

class sphere_list : public hittable
{
public:
  sphere_list() : hittable(nullptr) {}

  virtual bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const override;
  virtual bool get_bounding_box(aabb& out_box) const override;

  void clear() { objects.clear(); }
  void add(hittable* object) { objects.push_back(object); }
  void build_boxes();

public:
  std::vector<hittable*> objects;
};
