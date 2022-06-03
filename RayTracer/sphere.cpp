#include "stdafx.h"

#include "sphere.h"

bool sphere::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
  vec3 oc = r.origin - center;
  float a = r.direction.length_squared();
  float half_b = dot(oc, r.direction);
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

  rec.t = root;
  rec.p = r.at(rec.t);
  rec.normal = (rec.p - center) / radius;
  rec.material = material;

  return true;
}

bool sphere_list::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
  hit_record temp_rec;
  bool hit_anything = false;
  float closest_so_far = t_max;

  for (const sphere& object : objects)
  {
    if (object.hit(r, t_min, closest_so_far, temp_rec))
    {
      hit_anything = true;
      closest_so_far = temp_rec.t;
      rec = temp_rec;
    }
  }

  return hit_anything;
}