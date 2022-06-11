#pragma once

#include <cmath>
#include <iostream>

struct vec3
{
public:
  vec3() = default;
  vec3(float in_x, float in_y, float in_z) : e{ in_x, in_y, in_z } {}

  float operator[](int i) const { return e[i]; }
  float& operator[](int i) { return e[i]; }

  vec3 operator - () const { return vec3(-x, -y, -z); }

  vec3& operator += (const vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
  vec3& operator -= (const vec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
  vec3& operator *= (const vec3& v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
  vec3& operator /= (const vec3& v) { x /= v.x; y /= v.y; z /= v.z; return *this; }

  vec3& operator += (float t) { return *this += t; }
  vec3& operator -= (float t) { return *this -= t; }
  vec3& operator *= (float t) { return *this *= t; }
  vec3& operator /= (float t) { return *this /= t; }

  inline float length() const
  {
    return std::sqrt(length_squared());
  }

  inline float length_squared() const 
  {
    return x * x + y * y + z * z;
  }

public:
  union 
  {
    float e[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    struct { float x, y, z, padding; };
  };
};

inline vec3 operator + (const vec3& v, float t) { return vec3(v.x + t, v.y + t, v.z + t); }
inline vec3 operator - (const vec3& v, float t) { return vec3(v.x - t, v.y - t, v.z - t); }
inline vec3 operator * (const vec3& v, float t) { return vec3(v.x * t, v.y * t, v.z * t); }
inline vec3 operator / (const vec3& v, float t) { return vec3(v.x / t, v.y / t, v.z / t); }

inline vec3 operator + (float t, const vec3& v) { return v + t; }
inline vec3 operator - (float t, const vec3& v) { return v - t; }
inline vec3 operator * (float t, const vec3& v) { return v * t; }
inline vec3 operator / (float t, const vec3& v) { return v / t; }

inline vec3 operator + (const vec3& u, const vec3& v) { return vec3(u.x + v.x, u.y + v.y, u.z + v.z); }
inline vec3 operator - (const vec3& u, const vec3& v) { return vec3(u.x - v.x, u.y - v.y, u.z - v.z); }
inline vec3 operator * (const vec3& u, const vec3& v) { return vec3(u.x * v.x, u.y * v.y, u.z * v.z); }
inline vec3 operator / (const vec3& u, const vec3& v) { return vec3(u.x / v.x, u.y / v.y, u.z / v.z); }

inline std::ostream& operator<<(std::ostream& out, const vec3& v) { return out << '[' << v.x << ',' << v.y << ',' << v.z << ']'; }

inline vec3 unit_vector(vec3 v) { return v / v.length(); }

inline float dot(const vec3& u, const vec3& v) { return u.x * v.x + u.y * v.y + u.z * v.z; }
inline vec3 cross(const vec3& u, const vec3& v) { return vec3(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x); };
