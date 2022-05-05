#include "vec3.h"
#include "common.h"

std::ostream& operator<<(std::ostream& out, const vec3& v)
{
  return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

vec3 operator+(const vec3& u, const vec3& v)
{
  return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

vec3 operator-(const vec3& u, const vec3& v)
{
  return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

vec3 operator*(const vec3& u, const vec3& v)
{
  return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

vec3 operator*(float t, const vec3& v)
{
  return vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}

vec3 operator*(const vec3& v, float t)
{
  return t * v;
}

vec3 operator/(vec3 v, float t)
{
  return (1 / t) * v;
}


float dot(const vec3& u, const vec3& v)
{
  return u.e[0] * v.e[0] + u.e[1] * v.e[1] + u.e[2] * v.e[2];
}

vec3 cross(const vec3& u, const vec3& v)
{
  return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
    u.e[2] * v.e[0] - u.e[0] * v.e[2],
    u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

vec3 unit_vector(vec3 v)
{
  return v / v.length();
}

vec3 random()
{
  return vec3(random_float(), random_float(), random_float());
}

vec3 random(float min, float max)
{
  return vec3(random_float(min, max), random_float(min, max), random_float(min, max));
}

vec3 random_in_unit_sphere() 
{
  while (true) 
  {
    vec3 p = random(-1.0f, 1.0f);
    if (p.length_squared() >= 1)
    {
      continue;
    }
    return p;
  }
}