#pragma once

#include <string>

#include "ray.h"
#include "hittables.h"
#include "textures.h"


enum class material_type  // No RTTI, simple type detection
{
  none = 0,
  diffuse,
  texture,
  metal,
  dialectric,
  diffuse_light
};
static inline const char* material_type_names[] =
{
  "None",
  "Diffuse",
  "Texture",
  "Metal",
  "Dialectric",
  "Diffuse light"
};

class materials_collection
{
public:
  void add(material* instance);
  void remove(int instance_id);
  material* get_material(int index);
  std::vector<std::string> get_material_names();

private:
  std::vector<material*> instances;
};

class material
{
public:
  material() {}
  material(material_type type) : type(type) { }
  material(material_type type, std::string&& friendly_name) : type(type), friendly_name(friendly_name) { }
  virtual bool scatter(const ray& in_ray, const hit_record& in_rec, vec3& out_attenuation, ray& out_scattered) const;
  virtual vec3 emitted(float u, float v, const vec3& p) const;
  virtual void get_name(std::string& out_name) const;
  virtual void draw_edit_panel();

  material_type type = material_type::none;
  std::string friendly_name;
};


class diffuse_material : public material
{
public:
  diffuse_material() : material(material_type::diffuse) {}
  diffuse_material(const vec3& albedo, std::string&& friendly_name) : albedo(albedo), material(material_type::diffuse, std::move(friendly_name)) {}

  virtual bool scatter(const ray& in_ray, const hit_record& in_rec, vec3& out_attenuation, ray& out_scattered) const override;
  virtual void get_name(std::string& out_name) const;
  virtual void draw_edit_panel();

  vec3 albedo;
};


class texture_material : public material
{
public:
  texture_material() : material(material_type::texture) {}
  texture_material(texture* texture, std::string&& friendly_name) : texture(texture), material(material_type::texture, std::move(friendly_name)) {}

  virtual bool scatter(const ray& in_ray, const hit_record& in_rec, vec3& out_attenuation, ray& out_scattered) const override;
  virtual void get_name(std::string& out_name) const;
  virtual void draw_edit_panel();

  texture* texture = nullptr;
};


class metal_material : public material
{
public:
  metal_material() : material(material_type::metal) {}
  metal_material(const vec3& albedo, float fuzz, std::string&& friendly_name) : albedo(albedo), fuzz(fuzz), material(material_type::metal, std::move(friendly_name)) {}

  virtual bool scatter(const ray& in_ray, const hit_record& in_rec, vec3& out_attenuation, ray& out_scattered) const override;
  virtual void get_name(std::string& out_name) const;
  virtual void draw_edit_panel();

  vec3 albedo;
  float fuzz = 0.02f;
};


class dialectric_material : public material
{
public:
  dialectric_material() : material(material_type::dialectric) {}
  dialectric_material(float index_of_refraction, std::string&& friendly_name) : index_of_refraction(index_of_refraction), material(material_type::dialectric, std::move(friendly_name)) {}

  virtual bool scatter(const ray& in_ray, const hit_record& in_rec, vec3& out_attenuation, ray& out_scattered) const override;
  virtual void get_name(std::string& out_name) const;
  virtual void draw_edit_panel();

  float index_of_refraction = 1.5f;
};


class diffuse_light_material : public material
{
public:
  diffuse_light_material() : material(material_type::diffuse_light) {}
  diffuse_light_material(texture* texture, std::string&& friendly_name) : texture(texture), material(material_type::diffuse_light, std::move(friendly_name)) {}

  virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& out_attenuation, ray& out_scattered) const override;
  virtual vec3 emitted(float u, float v, const vec3& p) const override;
  virtual void get_name(std::string& out_name) const;
  virtual void draw_edit_panel();

public:
  texture* texture;
};