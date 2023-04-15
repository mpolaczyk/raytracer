#include "stdafx.h"

#include "processing/async_renderer_base.h"


nlohmann::json renderer_config::serialize()
{
  nlohmann::json j;
  to_json(j, *this);
  return j;
}

void renderer_config::deserialize(const nlohmann::json& j)
{
  TRY_PARSE(int, j, "rays_per_pixel", rays_per_pixel);
  TRY_PARSE(int, j, "ray_bounces", ray_bounces);
  TRY_PARSE(threading_strategy_type, j, "threading_strategy", threading_strategy);
  TRY_PARSE(bool, j, "reuse_buffer", reuse_buffer);
  TRY_PARSE(int, j, "resolution_vertical", resolution_vertical);
  TRY_PARSE(int, j, "resolution_horizontal", resolution_horizontal);
}