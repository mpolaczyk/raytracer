#include "stdafx.h"

#include "math/hittables.h"
#include "app/json/vec3_json.h"

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
  j["origin"] = vec3_serializer::serialize(origin);
  return j;
}

nlohmann::json scene::serialize()
{
  nlohmann::json jarr = nlohmann::json::array();
  std::transform(objects.begin(), objects.end(), std::back_inserter(jarr), [](hittable* object) { return object->serialize(); });
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

nlohmann::json static_mesh::serialize()
{
  nlohmann::json j;
  hittable::to_json(j, *this);
  to_json(j, *this);
  j["origin"] = vec3_serializer::serialize(origin);
  j["scale"] = vec3_serializer::serialize(scale);
  j["rotation"] = vec3_serializer::serialize(rotation);
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
  if (TRY_PARSE(nlohmann::json, j, "origin", jorigin)) { origin = vec3_serializer::deserialize(jorigin); }
}

void scene::deserialize(const nlohmann::json& j)
{
  hittable::from_json(j, *this);

  nlohmann::json jobjects;
  if (TRY_PARSE(nlohmann::json, j, "objects", jobjects))
  {
    for (const auto& element : jobjects)
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

void static_mesh::deserialize(const nlohmann::json& j)
{
  hittable::from_json(j, *this);

  TRY_PARSE(std::string, j, "file_name", file_name);
  TRY_PARSE(int32_t, j, "shape_index", shape_index);
  
  nlohmann::json jorigin;
  if (TRY_PARSE(nlohmann::json, j, "origin", jorigin)) { origin = vec3_serializer::deserialize(jorigin); }
  nlohmann::json jscale;
  if (TRY_PARSE(nlohmann::json, j, "scale", jscale)) { scale = vec3_serializer::deserialize(jscale); }
  nlohmann::json jrotation;
  if (TRY_PARSE(nlohmann::json, j, "rotation", jrotation)) { rotation = vec3_serializer::deserialize(jrotation); }
}