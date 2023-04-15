#include "stdafx.h"

#include "math/hittables.h"


nlohmann::json hittable::serialize()
{
  nlohmann::json j;
  to_json(j, *this);
  return j;
}

nlohmann::json sphere::serialize()
{
  nlohmann::json j;
  hittable::to_json(j, *this);
  to_json(j, *this);
  j["origin"] = origin.serialize();
  return j;
}

nlohmann::json scene::serialize()
{
  nlohmann::json jarr = nlohmann::json::array();
  for (hittable* object : objects)
  {
    jarr.push_back(object->serialize());
  }
  nlohmann::json j;
  hittable::to_json(j, *this);
  j["objects"] = jarr;
  return j;
}

nlohmann::json xy_rect::serialize()
{
  nlohmann::json j;
  hittable::to_json(j, *this);
  to_json(j, *this);
  return j;
}

nlohmann::json xz_rect::serialize()
{
  nlohmann::json j;
  hittable::to_json(j, *this);
  to_json(j, *this);
  return j;
}

nlohmann::json yz_rect::serialize()
{
  nlohmann::json j;
  hittable::to_json(j, *this);
  to_json(j, *this);
  return j;
}


void hittable::deserialize(const nlohmann::json& j)
{
  TRY_PARSE(hittable_class, j, "type", type);
  TRY_PARSE(std::string, j, "material_id", material_id);
}

void sphere::deserialize(const nlohmann::json& j)
{
  hittable::from_json(j, *this);
  
  TRY_PARSE(float, j, "radius", radius);

  nlohmann::json jorigin;
  if (TRY_PARSE(nlohmann::json, j, "origin", jorigin)) { origin.deserialize(jorigin); }
}

void scene::deserialize(const nlohmann::json& j)
{
  hittable::from_json(j, *this);

  nlohmann::json jobjects;
  if (TRY_PARSE(nlohmann::json, j, "objects", jobjects))
  {
    for (auto& element : jobjects)
    {
      hittable_class type;
      if (TRY_PARSE(hittable_class, element, "type", type))
      {
        hittable* obj = hittable::spawn_by_type(type);
        obj->deserialize(element);
        objects.push_back(obj);
      }
    }
  }
}

void xy_rect::deserialize(const nlohmann::json& j)
{
  hittable::from_json(j, *this);
  
  TRY_PARSE(float, j, "x0", x0);
  TRY_PARSE(float, j, "y0", y0);
  TRY_PARSE(float, j, "x1", x1);
  TRY_PARSE(float, j, "y1", y1);
  TRY_PARSE(float, j, "z", z);
}

void xz_rect::deserialize(const nlohmann::json& j)
{
  hittable::from_json(j, *this);

  TRY_PARSE(float, j, "x0", x0);
  TRY_PARSE(float, j, "z0", z0);
  TRY_PARSE(float, j, "x1", x1);
  TRY_PARSE(float, j, "z1", z1);
  TRY_PARSE(float, j, "y", y);
}

void yz_rect::deserialize(const nlohmann::json& j)
{
  hittable::from_json(j, *this);
  
  TRY_PARSE(float, j, "y0", y0);
  TRY_PARSE(float, j, "z0", z0);
  TRY_PARSE(float, j, "y1", y1);
  TRY_PARSE(float, j, "z1", z1);
  TRY_PARSE(float, j, "x", x);
}