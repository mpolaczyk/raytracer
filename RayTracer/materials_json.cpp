#include "stdafx.h"

#include "nlohmann\json.hpp"

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

nlohmann::json diffuse_material::serialize()
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
  from_json(j, *this);
}

void diffuse_material::deserialize(const nlohmann::json& j)
{
  material::from_json(j, *this);
  albedo.deserialize(j["albedo"]);
}

void texture_material::deserialize(const nlohmann::json& j)
{
  material::from_json(j, *this);
  // texture
  from_json(j, *this);
}

void metal_material::deserialize(const nlohmann::json& j)
{
  material::from_json(j, *this);
  albedo.deserialize(j["albedo"]);
  from_json(j, *this);
}

void dialectric_material::deserialize(const nlohmann::json& j)
{
  material::from_json(j, *this);
  from_json(j, *this);
}

void diffuse_light_material::deserialize(const nlohmann::json& j)
{
  material::from_json(j, *this);
  albedo.deserialize(j["albedo"]);
}

