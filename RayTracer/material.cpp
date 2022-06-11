#include "stdafx.h"

#include "material.h"

// Designated initializers c++20 https://en.cppreference.com/w/cpp/language/aggregate_initialization
material material::white_diffuse_preset
{
  .type = material_type::diffuse,
  .albedo = white_blue
};

material material::green_diffuse_preset
{
  .type = material_type::diffuse,
  .albedo = green
};

material material::yellow_diffuse_preset
{
  .type = material_type::diffuse,
  .albedo = yellow
};

material material::red_diffuse_preset
{
  .type = material_type::diffuse,
  .albedo = red
};

material material::metal_shiny_preset
{
  .type = material_type::metal_shiny,
  .albedo = grey
};

material material::glass_preset
{
  .type = material_type::dialectric,
  .albedo = grey
};

material material::metal_matt_preset
{
  .type = material_type::metal_matt,
  .albedo = grey
};

bool material::scatter(const ray& in_ray, const hit_record& in_rec, vec3& out_attenuation, ray& out_scattered) const
{
  // if else based material selection on purpose - much cheaper than OOP based approach
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
  else if (type == material_type::dialectric)
  {
    return scatter_dialectric(in_ray, in_rec, out_attenuation, out_scattered);
  }
}

bool material::scatter_diffuse(const ray& in_ray, const hit_record& in_hit, vec3& out_attenuation, ray& out_scattered) const
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

bool material::scatter_metal_shiny(const ray& in_ray, const hit_record& in_hit, vec3& out_attenuation, ray& out_scattered) const
{
  vec3 reflected = reflect(unit_vector(in_ray.direction), in_hit.normal);
  out_scattered = ray(in_hit.p, reflected);
  out_attenuation = albedo;
  return (dot(out_scattered.direction, in_hit.normal) > 0);
}

bool material::scatter_metal_matt(const ray& in_ray, const hit_record& in_hit, vec3& out_attenuation, ray& out_scattered) const
{
  float fuzz = 0.02f;
  vec3 reflected = reflect(unit_vector(in_ray.direction), in_hit.normal);
  out_scattered = ray(in_hit.p, reflected + fuzz * unit_vector(random_cache::get_vec3()));
  out_attenuation = albedo;
  return (dot(out_scattered.direction, in_hit.normal) > 0);
}

bool material::scatter_dialectric(const ray& in_ray, const hit_record& in_hit, vec3& out_attenuation, ray& out_scattered) const
{
  out_attenuation = vec3(1.0f, 1.0f, 1.0f);
  float ir = 1.5f;
  float refraction_ratio = in_hit.front_face ? (1.0f / ir) : ir;
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
