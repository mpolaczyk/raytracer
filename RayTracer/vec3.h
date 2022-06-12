#pragma once

#include <cmath>
#include <iostream>

#define USE_SIMD 0

#if USE_SIMD
#include <xmmintrin.h>
#endif

__declspec(align(16)) struct vec3
{
public:
  vec3() = default;
  vec3(float in_x, float in_y, float in_z) : e{ in_x, in_y, in_z } {}
#if USE_SIMD
  vec3(float f) { R128 = _mm_set_ps1(f); }
  vec3(const __m128& r128) { R128 = r128; }
#endif

  float operator[](int i) const { return e[i]; }
  float& operator[](int i) { return e[i]; }

  vec3 operator - () const { return vec3(-x, -y, -z); }

#if USE_SIMD
  vec3& operator += (const vec3& v) { R128 = _mm_add_ps(R128, v.R128); return *this; }
  vec3& operator -= (const vec3& v) { R128 = _mm_sub_ps(R128, v.R128); return *this; }
  vec3& operator *= (const vec3& v) { R128 = _mm_mul_ps(R128, v.R128); return *this; }
  vec3& operator /= (const vec3& v) { R128 = _mm_div_ps(R128, v.R128); return *this; }
  vec3& operator += (float t) { R128 = _mm_add_ps(R128, _mm_set_ps1(t)); return *this; }
  vec3& operator -= (float t) { R128 = _mm_sub_ps(R128, _mm_set_ps1(t)); return *this; }
  vec3& operator *= (float t) { R128 = _mm_mul_ps(R128, _mm_set_ps1(t)); return *this; }
  vec3& operator /= (float t) { R128 = _mm_div_ps(R128, _mm_set_ps1(t)); return *this; }
#else
  vec3& operator += (const vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
  vec3& operator -= (const vec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
  vec3& operator *= (const vec3& v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
  vec3& operator /= (const vec3& v) { x /= v.x; y /= v.y; z /= v.z; return *this; }
  vec3& operator += (float t) { return *this += t; }
  vec3& operator -= (float t) { return *this -= t; }
  vec3& operator *= (float t) { return *this *= t; }
  vec3& operator /= (float t) { return *this /= t; }
#endif

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
    __m128 R128;
  };
};

inline std::ostream& operator<<(std::ostream& out, const vec3& v) { return out << '[' << v.x << ',' << v.y << ',' << v.z << ']'; }

#if USE_SIMD
inline vec3 operator + (const vec3& v, float t) { return _mm_add_ps(v.R128, _mm_set_ps1(t)); }
inline vec3 operator - (const vec3& v, float t) { return _mm_sub_ps(v.R128, _mm_set_ps1(t)); }
inline vec3 operator * (const vec3& v, float t) { return _mm_mul_ps(v.R128, _mm_set_ps1(t)); }
inline vec3 operator / (const vec3& v, float t) { return _mm_div_ps(v.R128, _mm_set_ps1(t)); }
inline vec3 operator + (float t, const vec3& v) { return _mm_add_ps(v.R128, _mm_set_ps1(t)); }
inline vec3 operator - (float t, const vec3& v) { return _mm_sub_ps(v.R128, _mm_set_ps1(t)); }
inline vec3 operator * (float t, const vec3& v) { return _mm_mul_ps(v.R128, _mm_set_ps1(t)); }
inline vec3 operator / (float t, const vec3& v) { return _mm_div_ps(v.R128, _mm_set_ps1(t)); }
inline vec3 operator + (const vec3& u, const vec3& v) { return _mm_add_ps(u.R128, v.R128); }
inline vec3 operator - (const vec3& u, const vec3& v) { return _mm_sub_ps(u.R128, v.R128); }
inline vec3 operator * (const vec3& u, const vec3& v) { return _mm_mul_ps(u.R128, v.R128); }
inline vec3 operator / (const vec3& u, const vec3& v) { return _mm_div_ps(u.R128, v.R128); }
#else
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
#endif

inline vec3 unit_vector(vec3 v) { return v / v.length(); }
inline float dot(const vec3& u, const vec3& v) { return u.x * v.x + u.y * v.y + u.z * v.z; }
inline vec3 cross(const vec3& u, const vec3& v) { return vec3(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x); };

