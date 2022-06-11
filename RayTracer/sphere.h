#pragma once

#include <vector>

#include "ray.h"
#include "aabb.h"

class material;

struct hit_record 
{
  point3 p;
  vec3 normal;
  float t;
  material* material;
  bool front_face;
};

class sphere
{
public:
  sphere() {}
  sphere(point3 in_origin, float radius, material* material)
    : origin(in_origin), radius(radius), material(material)
  {
  };

  bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const;
  bool get_bounding_box(aabb& out_box) const;

public:
  // TODO: rethink sphere type
  point3 origin;
  float radius;
  material* material;
  aabb bounding_box;
};

class sphere_list
{
public:
  sphere_list() {}
  sphere_list(sphere object) { add(object); }

  void clear() { objects.clear(); }
  void add(sphere object) { objects.push_back(object); }

  bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const;
  bool get_bounding_box(aabb& out_box) const;
  void build_boxes();

public:
  std::vector<sphere> objects;
};
