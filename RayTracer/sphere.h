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
  material* material;
  bool front_face;
};

class sphere
{
public:
  sphere() {}
  sphere(vec3 in_origin, float radius, material* material)
    : origin(in_origin), radius(radius), material(material)
  {
  };

  bool hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const;
  bool get_bounding_box(aabb& out_box) const;

public:
  // TODO: rethink sphere type
  vec3 origin;
  float radius;
  material* material = nullptr;
  aabb bounding_box;

  static void get_sphere_uv(const vec3& p, float& out_u, float& out_v)
  {
    // p: a given point on the sphere of radius one, centered at the origin.
    // u: returned value [0,1] of angle around the Y axis from X=-1.
    // v: returned value [0,1] of angle from Y=-1 to Y=+1.
    //     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
    //     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
    //     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>
    float theta = acos(-p.y);
    float phi = atan2(-p.z, p.x) + pi;
    out_u = phi / (2.0f * pi);
    out_v = theta / pi;
  }
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
