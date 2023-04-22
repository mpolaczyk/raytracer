
#include "trace_ray.isph"

export uniform int render_chunk(const uniform vec3& resolution, const uniform chunk& ch)
{
  print("render_chunk: %d %d %d %d %d\n", ch.id, ch.x, ch.y, ch.size_x, ch.size_y);
  return resolution.x+1;
}