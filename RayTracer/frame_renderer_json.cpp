#include "stdafx.h"

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
  TRY_PARSE(int, j, "AA_samples_per_pixel", AA_samples_per_pixel);
  TRY_PARSE(int, j, "diffuse_max_bounce_num", diffuse_max_bounce_num);
  TRY_PARSE(float, j, "diffuse_bounce_brightness", diffuse_bounce_brightness);
  TRY_PARSE(int, j, "chunks_num", chunks_num);
  TRY_PARSE(chunk_strategy_type, j, "chunks_strategy", chunks_strategy);
  TRY_PARSE(bool, j, "shuffle_chunks", shuffle_chunks);
  TRY_PARSE(threading_strategy_type, j, "threading_strategy", threading_strategy);
  TRY_PARSE(int, j, "threads_num", threads_num);
  TRY_PARSE(bool, j, "allow_emissive", allow_emissive);
  TRY_PARSE(bool, j, "pixel_time_coloring", pixel_time_coloring);
  TRY_PARSE(float, j, "pixel_time_coloring_scale", pixel_time_coloring_scale);
  TRY_PARSE(bool, j, "reuse_buffer", reuse_buffer);
  TRY_PARSE(int, j, "resolution_vertical", resolution_vertical);
  TRY_PARSE(int, j, "resolution_horizontal", resolution_horizontal);

  nlohmann::json jbackground;
  if (TRY_PARSE(nlohmann::json, j, "background", jbackground)) { background.deserialize(jbackground); }
}