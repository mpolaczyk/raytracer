#pragma once

#include "ray.h"
#include "sphere.h"

enum class material_type
{
  none,
  diffuse,
  metal_shiny,
  metal_matt,
  dialectric
};

class material
{
public:

  static material white_diffuse_preset;
  static material green_diffuse_preset;
  static material yellow_diffuse_preset;
  static material red_diffuse_preset;
  static material metal_shiny_preset;
  static material glass_preset;
  static material metal_matt_preset;
  
  material_type type = material_type::none;
  vec3 albedo;

  bool scatter(const ray& in_ray, const hit_record& in_rec, vec3& out_attenuation, ray& out_scattered) const;

private:
  bool scatter_diffuse(const ray& in_ray, const hit_record& in_hit, vec3& out_attenuation, ray& out_scattered) const;
  bool scatter_metal_shiny(const ray& in_ray, const hit_record& in_hit, vec3& out_attenuation, ray& out_scattered) const;
  bool scatter_metal_matt(const ray& in_ray, const hit_record& in_hit, vec3& out_attenuation, ray& out_scattered) const;
  bool scatter_dialectric(const ray& in_ray, const hit_record& in_hit, vec3& out_attenuation, ray& out_scattered) const;
};