#include "stdafx.h"

#include "aabb.h"
#include "vec3.h"

bool aabb::hit(const ray& in_ray, float t_min, float t_max) const
{
#if 0 // USE_SIMD // commented out, non vectorized is faster in that case
  __m128 minod = _mm_div_ps(_mm_sub_ps(minimum.R128, in_ray.origin.R128), in_ray.direction.R128);
  __m128 maxod = _mm_div_ps(_mm_sub_ps(maximum.R128, in_ray.origin.R128), in_ray.direction.R128);
  vec3 t0 = vec3(_mm_min_ps(minod, maxod));
  vec3 t1 = vec3(_mm_max_ps(minod, maxod));
  for (int i = 0; i < 3; i++)
  {
    t_min = max1(t0[i], t_min);
    t_max = min1(t1[i], t_max);
    if (t_max < t_min)
    {
      return false;
    }
  }
  return true;
#else
  for (int i = 0; i < 3; i++)
  {
    float o = in_ray.origin[i];
    float d = in_ray.direction[i];
    float d_inv = 0.0f;
    {
      fpexcept::disabled_scope fpe;
      d_inv = 1 / in_ray.direction[i];  // this is allowed to produce 1/0 = inf
    }
    float t0 = math::min1((minimum[i] - o) * d_inv, (maximum[i] - o) * d_inv);
    float t1 = math::max1((minimum[i] - o) * d_inv, (maximum[i] - o) * d_inv);
    t_min = math::max1(t0, t_min);
    t_max = math::min1(t1, t_max);
    if (t_max <= t_min)
    {
      return false;
    }
  }
  return true;
#endif
}

bool aabb::hit2(const ray& in_ray, float t_min, float t_max) const
{
  // by Andrew Kensler at Pixar
  for (int a = 0; a < 3; a++)
  {
    float invD = 1.0f / in_ray.direction[a];
    float t0 = (minimum[a] - in_ray.origin[a]) * invD;
    float t1 = (maximum[a] - in_ray.origin[a]) * invD;
    if (invD < 0.0f)
    {
      std::swap(t0, t1);
    }
    t_min = t0 > t_min ? t0 : t_min;
    t_max = t1 < t_max ? t1 : t_max;
    if (t_max <= t_min)
    {
      return false;
    }
  }
  return true;
}

aabb aabb::merge(const aabb& box0, const aabb& box1)
{
#if USE_SIMD
  vec3 corner_min = vec3(_mm_min_ps(box0.minimum.R128, box1.minimum.R128));
  vec3 corner_max = vec3(_mm_min_ps(box0.maximum.R128, box1.maximum.R128));
#else
  vec3 corner_min(min1(box0.minimum.x, box1.minimum.x),
    min1(box0.minimum.y, box1.minimum.y),
    min1(box0.minimum.z, box1.minimum.z));
  vec3 corner_max(max1(box0.maximum.x, box1.maximum.x),
    max1(box0.maximum.y, box1.maximum.y),
    max1(box0.maximum.z, box1.maximum.z));
#endif
  return aabb(corner_min, corner_max);
}