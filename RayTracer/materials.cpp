#include "stdafx.h"

#include "materials.h"
#include "common.h"
#include "onb.h"

material* material::spawn_by_type(material_class type)
{
  if (type == material_class::dialectric) { return new dialectric_material(); }
  else if (type == material_class::lambertian) { return new lambertian_material(); }
  else if (type == material_class::isotropic) { return new isotropic_material(); }
  else if (type == material_class::diffuse_light) { return new diffuse_light_material(); }
  else if (type == material_class::metal) { return new metal_material(); }
  else if (type == material_class::texture) { return new texture_material(); }
  return nullptr;
}

bool material::scatter(const ray& in_ray, const hit_record& in_rec, vec3& out_attenuation, ray& out_scattered, float& out_pdf) const
{
  return false;
}

float material::scatter_pdf(const ray& in_ray, const hit_record& in_rec, const ray& in_scattered) const
{
  return 0.0f;
}

vec3 material::emitted(const hit_record& in_hit) const
{
  return vec3(0.0f, 0.0f, 0.0f);
}

bool lambertian_material::scatter(const ray& in_ray, const hit_record& in_hit, vec3& out_attenuation, ray& out_scattered, float& out_pdf) const
{
  // Lambertian - scatter direction uses a cosine distribution, so we need to rotate it to match the hit normal
  onb uvw;
  uvw.build_from_w(in_hit.normal);
  vec3 scatter_direction = uvw.local(random_cosine_direction());

  out_scattered = ray(in_hit.p, scatter_direction);
  out_attenuation = albedo;
  out_pdf = dot(uvw.w, out_scattered.direction) / pi;
  return true;
}

float lambertian_material::scatter_pdf(const ray& in_ray, const hit_record& in_hit, const ray& in_scattered) const
{
  float cosine = dot(in_hit.normal, unit_vector(in_scattered.direction));
  return cosine < 0.0f ? 0.0f : cosine / pi;
}

bool isotropic_material::scatter(const ray& in_ray, const hit_record& in_hit, vec3& out_attenuation, ray& out_scattered, float& out_pdf) const
{
  // Fake Lambertian, uniform distribution, bounces outside of the surface
  vec3 scatter_direction = random_unit_in_hemisphere(in_hit.normal);
  if (is_near_zero(scatter_direction))
  {
    scatter_direction = in_hit.normal;
  }
  out_scattered = ray(in_hit.p, scatter_direction);
  out_attenuation = albedo;
  out_pdf = 1.0f / (4.0f * pi);
  return true;
}

float isotropic_material::scatter_pdf(const ray& in_ray, const hit_record& in_hit, const ray& in_scattered) const
{
  return 1.0f / (4.0f * pi);
}

bool texture_material::scatter(const ray& in_ray, const hit_record& in_hit, vec3& out_attenuation, ray& out_scattered, float& out_pdf) const
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

bool metal_material::scatter(const ray& in_ray, const hit_record& in_hit, vec3& out_attenuation, ray& out_scattered, float& out_pdf) const
{
  vec3 reflected = reflect(unit_vector(in_ray.direction), in_hit.normal);
  out_scattered = ray(in_hit.p, reflected + fuzz * unit_vector(random_cache::get_vec3()));
  out_attenuation = albedo;
  return (dot(out_scattered.direction, in_hit.normal) > 0);
}

bool dialectric_material::scatter(const ray& in_ray, const hit_record& in_hit, vec3& out_attenuation, ray& out_scattered, float& out_pdf) const
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

bool diffuse_light_material::scatter(const ray& in_ray, const hit_record& in_hit, vec3& out_attenuation, ray& out_scattered, float& out_pdf) const
{
  return false;
}

vec3 diffuse_light_material::emitted(const hit_record& in_hit) const
{
  if (in_hit.front_face)
    return c_black;
  return albedo;
}
