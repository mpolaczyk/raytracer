#pragma once

#include <cmath>
#include <iostream>

using std::sqrt;

class vec3 
{
public:
  vec3() : e{ 0,0,0 } {}
  vec3(float e0, float e1, float e2) : e{ e0, e1, e2 } {}

  float inline x() const { return e[0]; }
  float inline y() const { return e[1]; }
  float inline z() const { return e[2]; }

  vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
  float operator[](int i) const { return e[i]; }
  float& operator[](int i) { return e[i]; }

  vec3& operator+=(const vec3& v) 
  {
    e[0] += v.e[0];
    e[1] += v.e[1];
    e[2] += v.e[2];
    return *this;
  }

  vec3& operator*=(const float t) 
  {
    e[0] *= t;
    e[1] *= t;
    e[2] *= t;
    return *this;
  }

  vec3& operator/=(const float t) 
  {
    return *this *= 1 / t;
  }

  vec3& operator-=(const float t)
  {
    return *this -= t;
  }

  vec3& operator+=(const float t)
  {
    return *this += t;
  }

  float length() const 
  {
    return sqrt(length_squared());
  }

  float length_squared() const 
  {
    return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
  }

public:
  float e[3];
};

// Type aliases for vec3
using point3 = vec3;   // 3D point
using color3 = vec3;    // RGB color

std::ostream& operator<<(std::ostream& out, const vec3& v);
vec3 operator+(const vec3& u, const vec3& v);
vec3 operator-(const vec3& u, const vec3& v);
vec3 operator*(const vec3& u, const vec3& v);
vec3 operator*(float t, const vec3& v);
vec3 operator*(const vec3& v, float t);
vec3 operator/(vec3 v, float t);
vec3 operator-(vec3 v, float t);
vec3 operator+(vec3 v, float t);

float dot(const vec3& u, const vec3& v);
vec3 cross(const vec3& u, const vec3& v);
vec3 unit_vector(vec3 v);
vec3 random();
vec3 random(float min, float max);
vec3 random_in_unit_sphere();