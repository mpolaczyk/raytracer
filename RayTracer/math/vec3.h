#pragma once

#include <cmath>
#include <iostream>

#include "app/json/serializable.h"

#define USE_SIMD 1
#if USE_SIMD
#include <intrin.h> // SSE3 required
#endif

__declspec(align(16)) class vec3
{
public:
  vec3() = default;
  vec3(float in_x, float in_y, float in_z) : e{ in_x, in_y, in_z } {}
#if USE_SIMD
  vec3(float f) { R128 = _mm_set_ps1(f); }
  vec3(const __m128& r128) { R128 = r128; }
#else
  vec3(float f) { x = f; y = f; z = f; }
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
  bool operator==(const vec3& v) { return e == v.e; }
  bool operator!=(const vec3& v) { return e == v.e; }

  inline float length() const
  {
#if USE_SIMD 
    __m128 a = _mm_mul_ps(R128, R128);
    a = _mm_hadd_ps(a, a);
    return _mm_cvtss_f32(_mm_sqrt_ps(_mm_hadd_ps(a, a)));
#else
    return std::sqrt(length_squared());
#endif
  }

  inline float length_squared() const 
  {
#if 0 // USE_SIMD // commented out, non vectorized is faster in that case
    __m128 a = _mm_mul_ps(R128, R128);
    a = _mm_hadd_ps(a, a);
    return _mm_cvtss_f32(_mm_hadd_ps(a,a));
#else
    return x * x + y * y + z * z;
#endif
  }

  inline bool is_zero() const
  {
    return x == 0.0f && y == 0.0f && z == 0.0f;
  }

  inline bool is_valid_color() const
  {
    return x <= 1.0f && y <= 1.0f && z <= 1.0f 
        && x >= 0.0f && y >= 0.0f && z >= 0.0f;
  }

public:
  union 
  {
    float e[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    struct { float x, y, z, padding; };
    __m128 R128;
  };

  uint32_t get_type_hash() const;
};

class vec3_serializer : serializable<nlohmann::json>
{
  // Separate serializer tool to reduce the sizeof(vec3) to 16 bytes
public:
  static nlohmann::json serialize(const vec3& value);
  static vec3 deserialize(const nlohmann::json& j);
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

inline vec3 unit_vector(const vec3& v)
{
#if USE_SIMD 
  __m128 a = _mm_mul_ps(v.R128, v.R128);
  a = _mm_hadd_ps(a, a);
  return _mm_div_ps(v.R128, _mm_sqrt_ps(_mm_hadd_ps(a, a)));
#else
  return v / v.length();
#endif
}

inline vec3 normalize(const vec3& v)
{
  return unit_vector(v);
}

inline float dot(const vec3& u, const vec3& v) 
{ 
#if USE_SIMD 
  __m128 a = _mm_mul_ps(u.R128, v.R128);
  a = _mm_hadd_ps(a, a);
  return _mm_cvtss_f32(_mm_hadd_ps(a, a));
#else
  return u.x * v.x + u.y * v.y + u.z * v.z;
#endif
}

inline vec3 cross(const vec3& u, const vec3& v) 
{ 
  return vec3(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x); 
};

inline vec3 rotate_yaw(const vec3& u, float yaw)
{
  float s = sinf(yaw);
  float c = cosf(yaw);
  return vec3(c * u.x - s * u.y, s * u.x + c * u.y, u.z);
}

inline vec3 rotate_pitch(const vec3& u, float pitch)
{
    float s = sinf(pitch);
    float c = cosf(pitch);
    return vec3(u.x, c * u.y - s * u.z, s * u.y + c * u.z);
}

inline vec3 rotate_roll(const vec3& u, float roll)
{
    float s = sinf(roll);
    float c = cosf(roll);
    return vec3(c * u.x - s * u.z, u.y, s * u.x + c * u.z);
}