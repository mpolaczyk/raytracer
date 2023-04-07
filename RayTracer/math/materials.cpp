#include "stdafx.h"

#include "materials.h"
#include "common.h"
#include "pdf.h"

material* material::spawn_by_type(material_class type)
{
  if (type == material_class::dialectric) { return new dialectric_material(); }
  else if (type == material_class::lambertian) { return new lambertian_material(); }
  else if (type == material_class::diffuse_light) { return new diffuse_light_material(); }
  else if (type == material_class::metal) { return new metal_material(); }
  else if (type == material_class::texture) { return new texture_material(); }
  return nullptr;
}

bool material::scatter(const ray& in_ray, const hit_record& in_rec, scatter_record& out_sr) const
{
  // return false it the ray was absorbed
  // return true if the ray was bounced/redirected
  return false;
}

float material::scatter_pdf(const ray& in_ray, const hit_record& in_rec, const ray& in_scattered) const
{
  return .0f;
}

vec3 material::emitted(const hit_record& in_hit) const
{
  return vec3(0.0f, 0.0f, 0.0f);
}

bool lambertian_material::scatter(const ray& in_ray, const hit_record& in_hit, scatter_record& out_sr) const
{
  out_sr.attenuation = albedo;
  out_sr.is_diffuse = true;
  return true;
}

float lambertian_material::scatter_pdf(const ray& in_ray, const hit_record& in_hit, const ray& in_scattered) const
{
  float cosine = dot(in_hit.normal, unit_vector(in_scattered.direction));
  return cosine < 0.0f ? 0.0f : cosine / pi;
}

bool texture_material::scatter(const ray& in_ray, const hit_record& in_hit, scatter_record& out_sr) const
{
  out_sr.attenuation = texture->value(in_hit.u, in_hit.v, in_hit.p);
  out_sr.is_diffuse = true;
  return true;
}

float texture_material::scatter_pdf(const ray& in_ray, const hit_record& in_hit, const ray& in_scattered) const
{
  float cosine = dot(in_hit.normal, unit_vector(in_scattered.direction));
  return cosine < 0.0f ? 0.0f : cosine / pi;
}

bool metal_material::scatter(const ray& in_ray, const hit_record& in_hit, scatter_record& out_sr) const
{
  out_sr.attenuation = albedo;
  out_sr.is_specular = true;
  vec3 reflected = reflect(unit_vector(in_ray.direction), in_hit.normal);
  out_sr.specular_ray = ray(in_hit.p, reflected + fuzz * random_in_unit_sphere());
  
  //return (dot(out_sr.specular_ray.direction, in_hit.normal) > 0);
  return true;
}

bool dialectric_material::scatter(const ray& in_ray, const hit_record& in_hit, scatter_record& out_sr) const
{
  out_sr.attenuation = vec3(1.0f, 1.0f, 1.0f);
  out_sr.is_specular = true;
  float refraction_ratio = in_hit.front_face ? (1.0f / index_of_refraction) : index_of_refraction;
  vec3 unit_direction = unit_vector(in_ray.direction);
  vec3 refracted = refract(unit_direction, in_hit.normal, refraction_ratio);
  out_sr.specular_ray = ray(in_hit.p, refracted);

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

vec3 diffuse_light_material::emitted(const hit_record& in_hit) const
{
  if (sides == (int)surface_side_type::aligned_to_normal && in_hit.front_face)
    return albedo;
  if (sides == (int)surface_side_type::opposite_to_normal && !in_hit.front_face)
    return albedo;
  if (sides == (int)surface_side_type::both)
    return albedo;
  return c_black;
}
