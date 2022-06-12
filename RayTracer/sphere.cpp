#include "stdafx.h"

#include "sphere.h"
#include "aabb.h"

bool sphere::hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const
{
  vec3 oc = in_ray.origin - origin;
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
  vec3 outward_normal = (out_hit.p - origin) / radius;
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
  get_sphere_uv(outward_normal, out_hit.u, out_hit.v);
  return true;
}

bool sphere::get_bounding_box(aabb& out_box) const
{
  out_box = aabb(origin - radius, origin + radius);
  return true;
}

bool sphere_list::hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const
{
  hit_record temp_rec;
  bool hit_anything = false;
  float closest_so_far = t_max;

  for (const sphere& object : objects)
  {
    // TODO: no hierarchical check, only replacement of the object hit function
    if (!object.bounding_box.hit(in_ray, t_min, t_max))
    {
      continue;
    }
    if (object.hit(in_ray, t_min, closest_so_far, temp_rec))
    {
      hit_anything = true;
      closest_so_far = temp_rec.t;
      out_hit = temp_rec;
    }
  }

  return hit_anything;
}

bool sphere_list::get_bounding_box(aabb& out_box) const
{
  if (objects.empty()) return false;

  aabb temp_box;
  bool first_box = true;

  for (const auto& object : objects)
  {
    if (!object.get_bounding_box(temp_box)) return false;
    out_box = first_box ? temp_box : aabb::merge(out_box, temp_box);
    first_box = false;
  }

  return true;
}

void sphere_list::build_boxes()
{
  for (sphere& object : objects)
  {
    object.get_bounding_box(object.bounding_box);
  }
}