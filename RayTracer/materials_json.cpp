#include "stdafx.h"

#include "materials.h"


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
  to_json(j, *this);
  return j;
}

nlohmann::json lambertian_material::serialize()
{
  nlohmann::json j;
  material::to_json(j, *this);
  j["albedo"] = albedo.serialize();
  return j;
}

nlohmann::json texture_material::serialize()
{
  nlohmann::json j;
  material::to_json(j, *this);
  //texture
  return j;
}

nlohmann::json metal_material::serialize()
{
  nlohmann::json j;
  material::to_json(j, *this);
  to_json(j, *this);
  j["albedo"] = albedo.serialize();
  return j;
}

nlohmann::json dialectric_material::serialize()
{
  nlohmann::json j;
  material::to_json(j, *this);
  to_json(j, *this);
  return j;
}

nlohmann::json diffuse_light_material::serialize()
{
  nlohmann::json j;
  material::to_json(j, *this);
  j["albedo"] = albedo.serialize();
  to_json(j, *this);
  return j;
}


void material_instances::deserialize(const nlohmann::json& j)
{
  for (auto& element : j)
  {
    material* obj = material::spawn_by_type(element["type"]);
    obj->deserialize(element);
    try_add(obj);
  }
}

void material::deserialize(const nlohmann::json& j)
{
  TRY_PARSE(material_class, j, "type", type);
  TRY_PARSE(std::string, j, "id", id);
}

void lambertian_material::deserialize(const nlohmann::json& j)
{
  material::from_json(j, *this);

  nlohmann::json jalbedo;
  if (TRY_PARSE(nlohmann::json, j, "albedo", jalbedo)) { albedo.deserialize(jalbedo); }
}

void texture_material::deserialize(const nlohmann::json& j)
{
  material::from_json(j, *this);
  // texture
}

void metal_material::deserialize(const nlohmann::json& j)
{
  material::from_json(j, *this);
  
  TRY_PARSE(float, j, "fuzz", fuzz);

  nlohmann::json jalbedo;
  if (TRY_PARSE(nlohmann::json, j, "albedo", jalbedo)) { albedo.deserialize(jalbedo); }
}

void dialectric_material::deserialize(const nlohmann::json& j)
{
  material::from_json(j, *this);
  
  TRY_PARSE(float, j, "index_of_refraction", index_of_refraction);
}

void diffuse_light_material::deserialize(const nlohmann::json& j)
{
  material::from_json(j, *this);

  TRY_PARSE(int, j, "sides", sides);

  nlohmann::json jalbedo;
  if (TRY_PARSE(nlohmann::json, j, "albedo", jalbedo)) { albedo.deserialize(jalbedo); }
}

