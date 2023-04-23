

#include "trace_ray.isph"

inline vec3 draw_uv(const int32 x, const int32 y, const uniform vec3& in_resolution)
{
  float u = (float)x / (float)(in_resolution.x - 1);
  float v = (float)y / (float)(in_resolution.y - 1);

  vec3 w = { u, v, 0.5f };
  return w;
}

inline vec3 draw_procedural_texture(const int32 x, const int32 y, const uniform vec3& in_resolution)
{
  vec3 v = { abs(sin(x / 50.0f)), abs(sin(y / 300.0f)), 0.5f };
  return v;
}

inline vec3 draw_random(const int32 x, const int32 y, const int32 index, const uniform vec3& in_resolution)
{
  return rand_direction(x * y + index);
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
    //output[index] = draw_uv(x, y, in_resolution);
    //output[index] = draw_procedural_texture(x, y, in_resolution);
    output[index] = draw_random(x, y, index, in_resolution);
  }
}