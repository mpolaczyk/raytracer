#include "stdafx.h"

#include "nlohmann\json.hpp"

#include "hittables.h"


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

nlohmann::json hittable_list::serialize()
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
  from_json(j, *this);
}

void sphere::deserialize(const nlohmann::json& j)
{
  hittable::from_json(j, *this);
  from_json(j, *this);
  origin.deserialize(j["origin"]);
}

void hittable_list::deserialize(const nlohmann::json& j)
{
  hittable::from_json(j, *this);
  for (auto& element : j["objects"])
  {
    hittable* obj = hittable::spawn_by_type(element["type"]);
    obj->deserialize(element);
    objects.push_back(obj);
  }
}

void xy_rect::deserialize(const nlohmann::json& j)
{
  hittable::from_json(j, *this);
  from_json(j, *this);
}

void xz_rect::deserialize(const nlohmann::json& j)
{
  hittable::from_json(j, *this);
  from_json(j, *this);
}

void yz_rect::deserialize(const nlohmann::json& j)
{
  hittable::from_json(j, *this);
  from_json(j, *this);
}