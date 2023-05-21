#pragma once

#include <string>
#include <map>

#include "ray.h"
#include "hittables.h"
#include "textures.h"
#include "pdf.h"
#include "app/json/serializable.h"
#include "app/factories.h"

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
  virtual nlohmann::json serialize() override;
  virtual void deserialize(const nlohmann::json& j) override;

private:
  std::map<std::string, material*> registry;
};

class material : serializable<nlohmann::json>
{
public:
  material() {}
  explicit material(material_type type) : type(type) 
  {
    if (type == material_type::light)
    {
      emitted_color = vec3(1.0f, 1.0f, 1.0f);
    }
  }
  material(std::string&& id, material_type type) : material(type)
  {
    id = std::move(id);
  }

  std::string id;
  material_type type = material_type::none;
  
  vec3 color;
  vec3 emitted_color;
  float smoothness = 0.0f;
  float gloss_probability = 0.0f;
  vec3 gloss_color;
  float refraction_probability = 0.0f;
  float refraction_index = 1.0f;

  void get_name(std::string& out_name, bool with_params=true) const;
  void draw_edit_panel();
  virtual nlohmann::json serialize() override;
  virtual void deserialize(const nlohmann::json& j) override;

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(material, type, id, smoothness, gloss_probability, refraction_probability, refraction_index); // to_json only
};