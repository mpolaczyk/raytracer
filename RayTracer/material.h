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
  material_type type = material_type::none;
  color3 albedo;

  bool scatter(const ray& in_ray, const hit_record& in_rec, color3& out_attenuation, ray& out_scattered) const;

private:
  bool scatter_diffuse(const ray& in_ray, const hit_record& in_rec, color3& out_attenuation, ray& out_scattered) const;
  bool scatter_metal_shiny(const ray& in_ray, const hit_record& in_rec, color3& out_attenuation, ray& out_scattered) const;
  bool scatter_metal_matt(const ray& in_ray, const hit_record& in_rec, color3& out_attenuation, ray& out_scattered) const;
  bool scatter_dialectric(const ray& in_ray, const hit_record& in_rec, color3& out_attenuation, ray& out_scattered) const;
};
