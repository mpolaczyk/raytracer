#include "stdafx.h"

#include "nlohmann\json.hpp"

#include "frame_renderer.h"


nlohmann::json renderer_config::serialize()
{
  nlohmann::json j;
  to_json(j, *this);
  j["background"] = background.serialize();
  return j;
}

void renderer_config::deserialize(const nlohmann::json& j)
{
  from_json(j, *this);
  background.deserialize(j["background"]);
}