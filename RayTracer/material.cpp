#include "stdafx.h"

#include "material.h"

bool material::scatter(const ray& in_ray, const hit_record& in_rec, color3& out_attenuation, ray& out_scattered) const
{
  if (type == material_type::diffuse)
  {
    return scatter_diffuse(in_ray, in_rec, out_attenuation, out_scattered);
  }
  else if (type == material_type::metal_shiny)
  {
    return scatter_metal_shiny(in_ray, in_rec, out_attenuation, out_scattered);
  }
  else if (type == material_type::metal_matt)
  {
    return scatter_metal_matt(in_ray, in_rec, out_attenuation, out_scattered);
  }
}

bool material::scatter_diffuse(const ray& in_ray, const hit_record& in_rec, color3& out_attenuation, ray& out_scattered) const
{
  // Fake Lambertian, uniform distribution, bounces outside of the surface
  vec3 scatter_direction = random_unit_in_hemisphere(in_rec.normal);

  if (is_near_zero(scatter_direction))
  {
    scatter_direction = in_rec.normal;
  }
  out_scattered = ray(in_rec.p, scatter_direction);
  out_attenuation = albedo;
  return true;
}

bool material::scatter_metal_shiny(const ray& in_ray, const hit_record& in_rec, color3& out_attenuation, ray& out_scattered) const
{
  vec3 reflected = reflect(unit_vector(in_ray.direction), in_rec.normal);
  out_scattered = ray(in_rec.p, reflected);
  out_attenuation = albedo;
  return (dot(out_scattered.direction, in_rec.normal) > 0);
}

bool material::scatter_metal_matt(const ray& in_ray, const hit_record& in_rec, color3& out_attenuation, ray& out_scattered) const
{
  float fuzz = 0.1f;
  vec3 reflected = reflect(unit_vector(in_ray.direction), in_rec.normal);
  out_scattered = ray(in_rec.p, reflected + fuzz * unit_vector(random_cache::get_vec3()));
  out_attenuation = albedo;
  return (dot(out_scattered.direction, in_rec.normal) > 0);
}