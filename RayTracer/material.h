#pragma once

#include "ray.h"
#include "sphere.h"

class material
{
public:
  virtual bool scatter(const ray& in_ray, const hit_record& in_rec, vec3& out_attenuation, ray& out_scattered) const;
};


class diffuse_material : public material
{
public:
  diffuse_material(const vec3& albedo) : albedo(albedo) {}

  virtual bool scatter(const ray& in_ray, const hit_record& in_rec, vec3& out_attenuation, ray& out_scattered) const override;

  vec3 albedo;
};


class metal_material : public material
{
public:
  metal_material(const vec3& albedo, float fuzz) : albedo(albedo), fuzz(fuzz) {}

  virtual bool scatter(const ray& in_ray, const hit_record& in_rec, vec3& out_attenuation, ray& out_scattered) const override;

  vec3 albedo;
  float fuzz = 0.02f;
};


class dialectric_material : public material
{
public:
  dialectric_material(float index_of_refraction) : index_of_refraction(index_of_refraction) {}

  virtual bool scatter(const ray& in_ray, const hit_record& in_rec, vec3& out_attenuation, ray& out_scattered) const override;

  float index_of_refraction = 1.5f;
};