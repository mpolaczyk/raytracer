#pragma once

#include "ray.h"
#include "sphere.h"
#include "texture.h"

class material
{
public:
  virtual bool scatter(const ray& in_ray, const hit_record& in_rec, vec3& out_attenuation, ray& out_scattered) const;
  virtual vec3 emitted(float u, float v, const vec3& p) const;
};


class diffuse_material : public material
{
public:
  diffuse_material(const vec3& albedo) : albedo(albedo) {}

  virtual bool scatter(const ray& in_ray, const hit_record& in_rec, vec3& out_attenuation, ray& out_scattered) const override;

  vec3 albedo;
};


class texture_material : public material
{
public:
  texture_material(texture* texture) : texture(texture) {}

  virtual bool scatter(const ray& in_ray, const hit_record& in_rec, vec3& out_attenuation, ray& out_scattered) const override;

  texture* texture = nullptr;
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


class diffuse_light_material : public material
{
public:
  diffuse_light_material(texture* texture) : texture(texture) {}

  virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& out_attenuation, ray& out_scattered) const override;
  virtual vec3 emitted(float u, float v, const vec3& p) const override;
  
public:
  texture* texture;
};