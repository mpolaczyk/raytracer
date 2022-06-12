#include "stdafx.h"

#include "material.h"
#include "common.h"

bool material::scatter(const ray& in_ray, const hit_record& in_rec, vec3& out_attenuation, ray& out_scattered) const
{
  return false;
}

bool diffuse_material::scatter(const ray& in_ray, const hit_record& in_hit, vec3& out_attenuation, ray& out_scattered) const
{
  // Fake Lambertian, uniform distribution, bounces outside of the surface
  vec3 scatter_direction = random_unit_in_hemisphere(in_hit.normal);
  if (is_near_zero(scatter_direction))
  {
    scatter_direction = in_hit.normal;
  }
  out_scattered = ray(in_hit.p, scatter_direction);
  out_attenuation = albedo;
  return true;
}

bool texture_material::scatter(const ray& in_ray, const hit_record& in_hit, vec3& out_attenuation, ray& out_scattered) const
{
  // Fake Lambertian, uniform distribution, bounces outside of the surface
  vec3 scatter_direction = random_unit_in_hemisphere(in_hit.normal);
  if (is_near_zero(scatter_direction))
  {
    scatter_direction = in_hit.normal;
  }
  out_scattered = ray(in_hit.p, scatter_direction);
  out_attenuation = texture->value(in_hit.u, in_hit.v, in_hit.p);
  return true;
}

bool metal_material::scatter(const ray& in_ray, const hit_record& in_hit, vec3& out_attenuation, ray& out_scattered) const
{
  vec3 reflected = reflect(unit_vector(in_ray.direction), in_hit.normal);
  out_scattered = ray(in_hit.p, reflected + fuzz * unit_vector(random_cache::get_vec3()));
  out_attenuation = albedo;
  return (dot(out_scattered.direction, in_hit.normal) > 0);
}

bool dialectric_material::scatter(const ray& in_ray, const hit_record& in_hit, vec3& out_attenuation, ray& out_scattered) const
{
  out_attenuation = vec3(1.0f, 1.0f, 1.0f);
  float refraction_ratio = in_hit.front_face ? (1.0f / index_of_refraction) : index_of_refraction;
  vec3 unit_direction = unit_vector(in_ray.direction);
  vec3 refracted = refract(unit_direction, in_hit.normal, refraction_ratio);
  out_scattered = ray(in_hit.p, refracted);

  // Total Internal Reflection and Schlick Approximation
  //float cos_theta = fmin(dot(-unit_direction, in_hit.normal), 1.0f);
  //float sin_theta = sqrt(1.0f - cos_theta * cos_theta);
  //bool cannot_refract = refraction_ratio * sin_theta > 1.0f;
  //vec3 direction;
  //if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_cache::get_float())
  //{
  //  direction = reflect(unit_direction, in_hit.normal);
  //}
  //else
  //{
  //  direction = refract(unit_direction, in_hit.normal, refraction_ratio);
  //}
  //out_scattered = ray(in_hit.p, direction);

  return true;
}
