#pragma once

#include <string>
#include <map>

#include "ray.h"
#include "hittables.h"
#include "textures.h"
#include "pdf.h"
#include "app/json/serializable.h"

enum class material_class  // No RTTI, simple type detection
{
  none = 0,
  universal
};
static inline const char* material_class_names[] =
{
  "None",
  "Universal"
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

  virtual vec3 get_color() const;
  virtual vec3 get_emitted() const;
  virtual float get_smoothness() const;
  virtual bool get_gloss_enabled() const;
  virtual float get_gloss_probability() const;
  virtual vec3 get_gloss_color() const;

  virtual void get_name(std::string& out_name, bool with_params=true) const;
  virtual void draw_edit_panel();
  virtual nlohmann::json serialize();
  virtual void deserialize(const nlohmann::json& j);

  material_class type = material_class::none;
  std::string id;

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(material, type, id); // to_json only

  static material* spawn_by_type(material_class type);
};

class universal_material : public material
{
public:
  universal_material() : material(material_class::universal) {}
  universal_material(std::string&& id) : material(std::move(id), material_class::universal) {}

  void get_name(std::string& out_name, bool with_params = true) const;
  void draw_edit_panel();
  nlohmann::json serialize();
  void deserialize(const nlohmann::json& j);

  material_class type = material_class::universal;
  std::string id;

  vec3 color;
  vec3 emitted_color;
  float smoothness = 0.0f;
  bool gloss_enabled = false;
  float gloss_probability = 0.0f;
  vec3 gloss_color;

  virtual vec3 get_color() const override { return color; }
  virtual vec3 get_emitted() const override { return emitted_color; }
  virtual float get_smoothness() const override { return smoothness; }
  virtual bool get_gloss_enabled() const override { return gloss_enabled; }
  virtual float get_gloss_probability() const override { return gloss_probability; }
  virtual vec3 get_gloss_color() const override { return gloss_color; }

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(universal_material, smoothness, gloss_enabled, gloss_probability); // to_json only
};