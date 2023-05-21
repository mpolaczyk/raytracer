#include "stdafx.h"

#include "math/materials.h"
#include "app/factories.h"
#include "app/json/vec3_json.h"

nlohmann::json material_instances::serialize()
{
  nlohmann::json jarr = nlohmann::json::array();
  for (std::pair<std::string, material*> pair : registry)
  {
    jarr.push_back(pair.second->serialize());
  }
  return jarr;
}

nlohmann::json material::serialize()
{
  nlohmann::json j;
  material::to_json(j, *this);
  j["color"] = vec3_serializer::serialize(color);
  j["emitted_color"] = vec3_serializer::serialize(emitted_color);
  j["gloss_color"] = vec3_serializer::serialize(gloss_color);
  to_json(j, *this);
  return j;
}


void material_instances::deserialize(const nlohmann::json& j)
{
  for (const auto& element : j)
  {
    material* obj = object_factory::spawn_material(element["type"]);
    obj->deserialize(element);
    try_add(obj);
  }
}

void material::deserialize(const nlohmann::json& j)
{
  TRY_PARSE(material_type, j, "type", type);
  TRY_PARSE(std::string, j, "id", id);

  nlohmann::json jcolor;
  if (TRY_PARSE(nlohmann::json, j, "color", jcolor)) { color = vec3_serializer::deserialize(jcolor); }
  assert(colors::is_valid(color));

  nlohmann::json jemitted_color;
  if (TRY_PARSE(nlohmann::json, j, "emitted_color", jemitted_color)) { emitted_color = vec3_serializer::deserialize(jemitted_color); }
  assert(colors::is_valid(emitted_color));

  TRY_PARSE(float, j, "smoothness", smoothness);
  assert(smoothness >= 0.0f && smoothness <= 1.0f);

  TRY_PARSE(float, j, "gloss_probability", gloss_probability);
  assert(gloss_probability >= 0.0f && gloss_probability <= 1.0f);

  nlohmann::json jgloss_color;
  if (TRY_PARSE(nlohmann::json, j, "gloss_color", jgloss_color)) { gloss_color = vec3_serializer::deserialize(jgloss_color); }
  assert(colors::is_valid(gloss_color));

  TRY_PARSE(float, j, "refraction_probability", refraction_probability);
  assert(refraction_probability >= 0.0f && refraction_probability <= 1.0f);
  TRY_PARSE(float, j, "refraction_index", refraction_index);  
}