#include "stdafx.h"

#include "hittables.h"
#include "aabb.h"

void hittable_list::build_boxes()
{
  for (hittable* object : objects)
  {
    object->get_bounding_box(object->bounding_box);
  }
}

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
  out_hit.mat = mat;

  // Normal always against the ray
  vec3 outward_normal = (out_hit.p - origin) / radius;
  out_hit.front_face = flip_normal_if_front_face(in_ray.direction, outward_normal, out_hit.normal);
  get_sphere_uv(outward_normal, out_hit.u, out_hit.v);
  return true;
}

bool hittable_list::hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const
{
  hit_record temp_rec;
  bool hit_anything = false;
  float closest_so_far = t_max;

  for (const hittable* object : objects)
  {
    // TODO: no hierarchical check, only replacement of the object hit function
    if (!object->bounding_box.hit(in_ray, t_min, t_max))
    {
      continue;
    }
    if (object->hit(in_ray, t_min, closest_so_far, temp_rec))
    {
      hit_anything = true;
      closest_so_far = temp_rec.t;
      out_hit = temp_rec;
    }
  }

  return hit_anything;
}

bool xy_rect::hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const
{
  float t = (z - in_ray.origin.z) / in_ray.direction.z;
  if (t < t_min || t > t_max) { return false; }
  float x = in_ray.origin.x + t * in_ray.direction.x;
  float y = in_ray.origin.y + t * in_ray.direction.y;
  if (x < x0 || x > x1 || y < y0 || y > y1) { return false; }
  out_hit.u = (x - x0) / (x1 - x0);
  out_hit.v = (y - y0) / (y1 - y0);
  out_hit.t = t;
  out_hit.front_face = flip_normal_if_front_face(in_ray.direction, vec3(0.0f, 0.0f, 1.0f), out_hit.normal);
  out_hit.mat = mat;
  out_hit.p = in_ray.at(t);
  return true;
}

bool xz_rect::hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const
{
  float t = (y - in_ray.origin.y) / in_ray.direction.y;
  if (t < t_min || t > t_max) { return false; }
  float x = in_ray.origin.x + t * in_ray.direction.x;
  float z = in_ray.origin.z + t * in_ray.direction.z;
  if (x < x0 || x > x1 || z < z0 || z > z1) { return false; }
  out_hit.u = (x - x0) / (x1 - x0);
  out_hit.v = (z - z0) / (z1 - z0);
  out_hit.t = t;
  out_hit.front_face = flip_normal_if_front_face(in_ray.direction, vec3(0.0f, 1.0f, 0.0f), out_hit.normal);
  out_hit.mat = mat;
  out_hit.p = in_ray.at(t);
  return true;
}

bool yz_rect::hit(const ray& in_ray, float t_min, float t_max, hit_record& out_hit) const
{
  float t = (x - in_ray.origin.x) / in_ray.direction.x;
  if (t < t_min || t > t_max) { return false; }
  float y = in_ray.origin.y + t * in_ray.direction.y;
  float z = in_ray.origin.z + t * in_ray.direction.z;
  if (y < y0 || y > y1 || z < z0 || z > z1) { return false; }
  out_hit.u = (y - y0) / (y1 - y0);
  out_hit.v = (z - z0) / (z1 - z0);
  out_hit.t = t;
  out_hit.front_face = flip_normal_if_front_face(in_ray.direction, vec3(1.0f, 0.0f, 0.0f), out_hit.normal);
  out_hit.mat = mat;
  out_hit.p = in_ray.at(t);
  return true;
}


bool sphere::get_bounding_box(aabb& out_box) const
{
  out_box = aabb(origin - radius, origin + radius);
  return true;
}

bool hittable_list::get_bounding_box(aabb& out_box) const
{
  if (objects.empty()) return false;

  aabb temp_box;
  bool first_box = true;

  for (const hittable* object : objects)
  {
    if (!object->get_bounding_box(temp_box)) return false;
    out_box = first_box ? temp_box : aabb::merge(out_box, temp_box);
    first_box = false;
  }

  return true;
}

bool xy_rect::get_bounding_box(aabb& out_box) const
{
  // The bounding box must have non-zero width in each dimension, so pad the Z
  // dimension a small amount.
  out_box = aabb(vec3(x0, y0, z - 0.0001f), vec3(x1, y1, z + 0.0001f));
  return true;
}

bool xz_rect::get_bounding_box(aabb& out_box) const
{
  // The bounding box must have non-zero width in each dimension, so pad the Y
  // dimension a small amount.
  out_box = aabb(vec3(x0, y - 0.0001, z0), vec3(x1, y + 0.0001, z1));
  return true;
}

bool yz_rect::get_bounding_box(aabb& out_box) const
{
  // The bounding box must have non-zero width in each dimension, so pad the X
  // dimension a small amount.
  out_box = aabb(vec3(x - 0.0001, y0, z0), vec3(x + 0.0001, y1, z1));
  return true;
}