#pragma once

#include <limits>
#include <random>

#include "vec3.h"

const vec3 c_white = vec3(0.73f, .73f, .73f);
const vec3 c_grey = vec3(0.6f, 0.6f, 0.6f);
const vec3 c_black = vec3(0.0f, 0.0f, 0.0f);
const vec3 c_red = vec3(0.65f, 0.05f, 0.05f);
const vec3 c_green = vec3(.12f, .45f, .15f);
const vec3 c_blue = vec3(0.0f, 0.0f, 1.0f);
const vec3 c_white_blue = vec3(0.5f, 0.7f, 1.0f);
const vec3 c_yellow = vec3(1.0f, 1.0f, 0.0f);
const vec3 c_copper = vec3(0.72f, 0.45f, 0.2f);
const vec3 c_steel = vec3(0.44f, 0.47f, 0.49f);
const vec3 c_silver = vec3(0.32f, 0.34f, 0.34f);
const vec3 c_gold = vec3(1.f, 0.84f, 0.f);

const float infinity = std::numeric_limits<float>::infinity();
const float pi = 3.1415926535897932385f;
const float small_number = 1e-8;

float degrees_to_radians(float degrees);
float sign(float value);
vec3 random_unit_in_hemisphere(const vec3& normal);
vec3 random_in_unit_disk();
bool is_near_zero(vec3& value);
vec3 reflect(const vec3& v, const vec3& n);
vec3 refract(const vec3& uv, const vec3& n, float etai_over_etat);
float reflectance(float cosine, float ref_idx);
bool flip_normal_if_front_face(const vec3& in_ray_direction, const vec3& in_outward_normal, vec3& out_normal);
inline float lerp_float(float a, float b, float f) { return a + f * (b - a); }
inline float min1(float a, float b) { return a < b ? a : b; }
inline float max1(float a, float b) { return a < b ? b : a; }
inline float clamp(float a, float b, float f) { return  min1(b, max1(a, f)); }
inline vec3 min3(const vec3& a, const vec3& b)
{
  return vec3(min1(a[0], b[0]), min1(a[1], b[1]), min1(a[2], b[2]));
}
inline vec3 max3(const vec3& a, const vec3& b)
{
  return vec3(max1(a[0], b[0]), max1(a[1], b[1]), max1(a[2], b[2]));
}
inline void get_sphere_uv(const vec3& p, float& out_u, float& out_v)
{
  // p: a given point on the sphere of radius one, centered at the origin.
  // u: returned value [0,1] of angle around the Y axis from X=-1.
  // v: returned value [0,1] of angle from Y=-1 to Y=+1.
  //     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
  //     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
  //     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>
  float theta = acos(-p.y);
  float phi = atan2(-p.z, p.x) + pi;
  out_u = phi / (2.0f * pi);
  out_v = theta / pi;
}

namespace random_cache
{
  static int num = 500000;
  static int last_index = 0;
  static std::vector<float> cache;
  static std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);

  void init();
  float get_float();
  vec3 get_vec3();
}

inline uint32_t hash_combine(uint32_t A, uint32_t C)
{
  uint32_t B = 0x9e3779b9;
  A += B;

  A -= B; A -= C; A ^= (C >> 13);
  B -= C; B -= A; B ^= (A << 8);
  C -= A; C -= B; C ^= (B >> 13);
  A -= B; A -= C; A ^= (C >> 12);
  B -= C; B -= A; B ^= (A << 16);
  C -= A; C -= B; C ^= (B >> 5);
  A -= B; A -= C; A ^= (C >> 3);
  B -= C; B -= A; B ^= (A << 10);
  C -= A; C -= B; C ^= (B >> 15);

  return C;
}

inline uint32_t hash_combine(uint32_t A, uint32_t B, uint32_t C, uint32_t D = 0)
{
  return hash_combine(hash_combine(A, B), hash_combine(C, D));
}

inline uint32_t pointer_hash(const void* ptr, uint32_t C = 0)
{
  uint32_t ptr_int = reinterpret_cast<size_t>(ptr);

  return hash_combine(ptr_int, C);
}


//inline uint32_t get_type_hash(const uint8_t A)
//{
//  return A;
//}
//
//inline uint32_t get_type_hash(const int8_t A)
//{
//  return A;
//}
//
//inline uint32_t get_type_hash(const uint16_t A)
//{
//  return A;
//}
//
//inline uint32_t get_type_hash(const int16_t A)
//{
//  return A;
//}
//
//inline uint32_t get_type_hash(const int32_t A)
//{
//  return A;
//}
//
//inline uint32_t get_type_hash(const uint32_t A)
//{
//  return A;
//}

inline uint32_t get_type_hash(const uint64_t A)
{
  return (uint32_t)A + ((uint32_t)(A >> 32) * 23);
}

inline uint32_t get_type_hash(const int64_t A)
{
  return (uint32_t)A + ((uint32_t)(A >> 32) * 23);
}

inline uint32_t get_type_hash(float Value)
{
  return *(uint32_t*)&Value;
}

inline uint32_t get_type_hash(double Value)
{
  return get_type_hash(*(uint64_t*)&Value);
}

inline uint32_t get_type_hash(const void* A)
{
  return pointer_hash(A);
}

inline uint32_t get_type_hash(void* A)
{
  return pointer_hash(A);
}

inline uint32_t get_type_hash(bool Value)
{
  return (uint32_t)Value;
}