

#include "trace_ray.isph"

inline vec3 draw_uv_noise(varying int32 x, varying int32 y, const uniform vec3& in_resolution)
{
  float u = (float)x / (float)(in_resolution.x - 1);
  float v = (float)y / (float)(in_resolution.y - 1);

  return set_vec3(u, v, 0.5f);
}

inline vec3 draw_procedural_texture(varying int32 x, varying int32 y, const uniform vec3& in_resolution)
{
  return set_vec3(abs(sin(x / 50.0f)), abs(sin(y / 300.0f)), 0.5f);
}

inline vec3 draw_random(varying int32 x, varying int32 y, const uniform vec3& in_resolution)
{
  return rand_direction(in_resolution.x * in_resolution.y);
}


export void render_chunk(const uniform vec3& in_resolution, const uniform chunk& in_chunk, uniform vec3 output[])
{
  const uniform int32 y_min = in_chunk.y;
  const uniform int32 y_max = in_chunk.y + in_chunk.size_y;
  const uniform int32 x_min = in_chunk.x;
  const uniform int32 x_max = in_chunk.x + in_chunk.size_x;

  foreach_tiled(y = y_min ... y_max, x = x_min ... x_max)
  {
    const int32 index = y + x * in_chunk.size_y;
    output[index] = draw_random(x, y, in_resolution);
  }
}