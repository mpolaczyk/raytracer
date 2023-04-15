#include "stdafx.h"

#include "math/materials.h"
#include "app/factories.h"


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
  j["color"] = color.serialize();
  j["emitted_color"] = emitted_color.serialize();
  j["gloss_color"] = gloss_color.serialize();
  to_json(j, *this);
  return j;
}


void material_instances::deserialize(const nlohmann::json& j)
{
  for (auto& element : j)
  {
    material* obj = object_factory::spawn_material(element["type"]);
    obj->deserialize(element);
    try_add(obj);
  }
}

void material::deserialize(const nlohmann::json& j)
{
  TRY_PARSE(material_class, j, "type", type);
  TRY_PARSE(std::string, j, "id", id);

  nlohmann::json jcolor;
  if (TRY_PARSE(nlohmann::json, j, "color", jcolor)) { color.deserialize(jcolor); }

  nlohmann::json jemitted_color;
  if (TRY_PARSE(nlohmann::json, j, "emitted_color", jemitted_color)) { emitted_color.deserialize(jemitted_color); }

  TRY_PARSE(float, j, "smoothness", smoothness);
  TRY_PARSE(bool, j, "gloss_enabled", gloss_enabled);
  TRY_PARSE(float, j, "gloss_probability", gloss_probability);

  nlohmann::json jgloss_color;
  if (TRY_PARSE(nlohmann::json, j, "gloss_color", jgloss_color)) { gloss_color.deserialize(jgloss_color); }
}