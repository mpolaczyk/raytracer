#pragma once

#include <map>

#include "app/factories.h"

class material
{
public:
  material() = default;
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
  ~material() = default;

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
};

class material_instances
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

  std::map<std::string, material*> registry;
};