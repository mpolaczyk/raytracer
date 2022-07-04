#include "stdafx.h"

#include "nlohmann\json.hpp"

#include "app.h"


nlohmann::json window_config::serialize()
{
  nlohmann::json j;
  to_json(j, *this);
  return j;
}

void window_config::deserialize(const nlohmann::json& j)
{
  from_json(j, *this);
}