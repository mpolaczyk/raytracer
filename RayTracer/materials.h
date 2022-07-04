#pragma once

#include <string>

#include "ray.h"
#include "hittables.h"
#include "textures.h"
#include <map>

#include "serializable.h"

enum class material_class  // No RTTI, simple type detection
{
  none = 0,
  diffuse,
  texture,
  metal,
  dialectric,
  diffuse_light
};
static inline const char* material_class_names[] =
{
  "None",
  "Diffuse",
  "Texture",
  "Metal",
  "Dialectric",
  "Diffuse light"
};

class material_instances : serializable<nlohmann::json>
{
public:
  bool is_id_in_use(const std::string& id) const;
  bool try_add(material* instance);
  void remove(const std::string& id);
  material* get_material(const std::string& id) const;
  std::vector<std::string> get_material_ids() const;
  std::vector<std::string> get_material_names() const;
  int get_index_by_name(const std::string& name) const;
  int get_index_by_id(const std::string& id) const;
  nlohmann::json serialize();
  void deserialize(const nlohmann::json& j);

private:
  std::map<std::string, material*> registry;
};

class material : serializable<nlohmann::json>
{
public:
  material() {}
  material(material_class type) : type(type) { }
  material(std::string&& id, material_class type) : id(std::move(id)), type(type) { }
  virtual bool scatter(const ray& in_ray, const hit_record& in_rec, vec3& out_attenuation, ray& out_scattered) const;
  virtual vec3 emitted(float u, float v, const vec3& p) const;
  virtual void get_name(std::string& out_name, bool with_params=true) const;
  virtual void draw_edit_panel();
  virtual nlohmann::json serialize();
  virtual void deserialize(const nlohmann::json& j);

  material_class type = material_class::none;
  std::string id;

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(material, type, id); // to_json only

  static material* spawn_by_type(material_class type);
};


class diffuse_material : public material
{
public:
  diffuse_material() : material(material_class::diffuse) {}
  diffuse_material(std::string&& id, const vec3& albedo) : albedo(albedo), material(std::move(id), material_class::diffuse) {}

  virtual bool scatter(const ray& in_ray, const hit_record& in_rec, vec3& out_attenuation, ray& out_scattered) const override;
  virtual void get_name(std::string& out_name, bool with_params) const;
  virtual void draw_edit_panel();
  virtual nlohmann::json serialize() override;
  virtual void deserialize(const nlohmann::json& j) override;

  vec3 albedo;
};


class texture_material : public material
{
public:
  texture_material() : material(material_class::texture) {}
  texture_material(std::string&& id, texture* texture) : texture(texture), material(std::move(id), material_class::texture) {}

  virtual bool scatter(const ray& in_ray, const hit_record& in_rec, vec3& out_attenuation, ray& out_scattered) const override;
  virtual void get_name(std::string& out_name, bool with_params) const;
  virtual void draw_edit_panel();
  virtual nlohmann::json serialize() override;
  virtual void deserialize(const nlohmann::json& j) override;

  texture* texture = nullptr;
};


class metal_material : public material
{
public:
  metal_material() : material(material_class::metal) {}
  metal_material(std::string&& id, const vec3& albedo, float fuzz) : albedo(albedo), fuzz(fuzz), material(std::move(id), material_class::metal) {}

  virtual bool scatter(const ray& in_ray, const hit_record& in_rec, vec3& out_attenuation, ray& out_scattered) const override;
  virtual void get_name(std::string& out_name, bool with_params) const;
  virtual void draw_edit_panel();
  virtual nlohmann::json serialize() override;
  virtual void deserialize(const nlohmann::json& j) override;

  vec3 albedo;
  float fuzz = 0.02f;

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(metal_material, fuzz); // to_json only
};


class dialectric_material : public material
{
public:
  dialectric_material() : material(material_class::dialectric) {}
  dialectric_material(std::string&& id, float index_of_refraction) : index_of_refraction(index_of_refraction), material(std::move(id), material_class::dialectric) {}

  virtual bool scatter(const ray& in_ray, const hit_record& in_rec, vec3& out_attenuation, ray& out_scattered) const override;
  virtual void get_name(std::string& out_name, bool with_params) const;
  virtual void draw_edit_panel();
  virtual nlohmann::json serialize() override;
  virtual void deserialize(const nlohmann::json& j) override;

  float index_of_refraction = 1.5f;

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(dialectric_material, index_of_refraction); // to_json only
};


class diffuse_light_material : public material
{
public:
  diffuse_light_material() : material(material_class::diffuse_light) {}
  diffuse_light_material(std::string&& id, vec3 albedo) : albedo(albedo), material(std::move(id), material_class::diffuse_light) {}

  virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& out_attenuation, ray& out_scattered) const override;
  virtual vec3 emitted(float u, float v, const vec3& p) const override;
  virtual void get_name(std::string& out_name, bool with_params) const;
  virtual void draw_edit_panel();
  virtual nlohmann::json serialize() override;
  virtual void deserialize(const nlohmann::json& j) override;

public:
  vec3 albedo;
};