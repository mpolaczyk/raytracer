#pragma once

#include <vector>

#include "ray.h"

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
  sphere(point3 cen, float r, material* material) 
    : center(cen), radius(r), material(material)
  {};

  bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const;

public:
  // TODO: rethink sphere type
  point3 center;
  float radius;
  material* material;
};

class sphere_list
{
public:
  sphere_list() {}
  sphere_list(sphere object) { add(object); }

  void clear() { objects.clear(); }
  void add(sphere object) { objects.push_back(object); }

  bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const;

public:
  std::vector<sphere> objects;
};
