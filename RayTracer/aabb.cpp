#include "stdafx.h"

#include "aabb.h"

bool aabb::hit(const ray& in_ray, float t_min, float t_max) const
{
  for (int i = 0; i < 3; i++)
  {
    float o = in_ray.origin[i];
    float d = in_ray.direction[i];
    float t0 = min2((minimum[i] - o) / d, (maximum[i] - o) / d);
    float t1 = max2((minimum[i] - o) / d, (maximum[i] - o) / d);
    t_min = max2(t0, t_min);
    t_max = min2(t1, t_max);
    if (t_max <= t_min)
    {
      return false;
    }
  }
  return true;
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
  point3 corner_min(min2(box0.minimum.x(), box1.minimum.x()),
    min2(box0.minimum.y(), box1.minimum.y()),
    min2(box0.minimum.z(), box1.minimum.z()));

  point3 corner_max(max2(box0.maximum.x(), box1.maximum.x()),
    max2(box0.maximum.y(), box1.maximum.y()),
    max2(box0.maximum.z(), box1.maximum.z()));

  return aabb(corner_min, corner_max);
}