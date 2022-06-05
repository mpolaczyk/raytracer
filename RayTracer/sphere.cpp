#include "stdafx.h"

#include "sphere.h"

bool sphere::hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const
{
  vec3 oc = in_ray.origin - center;
  float a = in_ray.direction.length_squared();
  float half_b = dot(oc, in_ray.direction);
  float c = oc.length_squared() - radius * radius;

  float delta = half_b * half_b - a * c;
  if (delta < 0.0f)
  {
    return false;
  }

  // Find the nearest root that lies in the acceptable range.
  float sqrtd = sqrt(delta);
  float root = (-half_b - sqrtd) / a;
  if (root < t_min || t_max < root)
  {
    root = (-half_b + sqrtd) / a;
    if (root < t_min || t_max < root)
    {
      return false;
    }
  }

  out_hit.t = root;
  out_hit.p = in_ray.at(out_hit.t);
  out_hit.material = material;

  // Normal always against the ray
  vec3 outward_normal = (out_hit.p - center) / radius;
  if (dot(in_ray.direction, outward_normal) < 0)
  {
    // Ray is inside
    out_hit.normal = outward_normal;
    out_hit.front_face = true;
  }
  else
  {
    // Ray is outside
    out_hit.normal = -outward_normal;
    out_hit.front_face = false;
  }
  return true;
}

bool sphere_list::hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const
{
  hit_record temp_rec;
  bool hit_anything = false;
  float closest_so_far = t_max;

  for (const sphere& object : objects)
  {
    if (object.hit(in_ray, t_min, closest_so_far, temp_rec))
    {
      hit_anything = true;
      closest_so_far = temp_rec.t;
      out_hit = temp_rec;
    }
  }

  return hit_anything;
}