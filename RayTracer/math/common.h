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
const float small_number = 0.00000001f;
constexpr float epsilon = std::numeric_limits<float>::epsilon();

inline float sign(float value)
{
  return value >= 0.0f ? 1.0f : -1.0f;  //  Assume 0 is positive
}
inline float degrees_to_radians(float degrees)
{
  return degrees * pi / 180.0f;
}
inline bool is_almost_zero(float value)
{
  return value <= epsilon && value >= -epsilon;
}
inline bool is_almost_equal(float a, float b)
{
  return fabs(a - b) <= epsilon;
}
inline bool is_near_zero(vec3& value)
{
  return (fabs(value[0]) < small_number) && (fabs(value[1]) < small_number) && (fabs(value[2]) < small_number);
}

inline float inv_sqrt(float x) 
{
  float xhalf = 0.5f * x;
  int i = *(int*)&x;            // store floating-point bits in integer
  i = 0x5f3759df - (i >> 1);    // initial guess for Newton's method
  x = *(float*)&i;              // convert new bits into float
  x = x * (1.5f - xhalf * x * x);     // One round of Newton's method
  return x;
}

namespace random_cache
{
  template<typename T, int N>
  struct cache
  {
    T get();
    void add(T value);
    int32_t len();

  private:
    int last_index = 0;
    std::vector<T> storage;
  };

  static cache<float, 500000> float_cache; // Range: [-1, 1]
  static cache<vec3, 50000> cosine_direction_cache;
  void init();

  float get_float();
  float get_float_0_1();
  float get_float_0_N(float N);
  float get_float_M_N(float M, float N);

  vec3 get_vec3();
  vec3 get_vec3_0_1();

  int32_t get_int_0_N(int32_t N);

  vec3 get_cosine_direction();
}

// Random function compendium: https://www.shadertoy.com/view/XlGcRh
float rand_iqint1(uint32_t seed);
float rand_pcg(uint32_t seed);

#define RAND_SEED_FUNC(seed) rand_pcg(seed)

vec3 rand_direction();
vec3 rand_direction(uint32_t seed);

float rand_normal_distribution();
float rand_normal_distribution(uint32_t seed);


inline vec3 random_in_unit_disk()
{
  vec3 dir = normalize(random_cache::get_vec3());
  return dir * random_cache::get_float();
}
inline vec3 random_in_unit_disk(uint32_t seed)
{
  vec3 dir = normalize(RAND_SEED_FUNC(seed));
  return dir * RAND_SEED_FUNC(seed);
}

inline vec3 random_unit_in_hemisphere(const vec3& normal)
{
  vec3 dir = normalize(random_cache::get_vec3());
  return dir * sign(dot(dir, normal));
}
inline vec3 random_unit_in_hemisphere(const vec3& normal, uint32_t seed)
{
  vec3 dir = rand_direction(seed);
  return dir * sign(dot(normal, dir));
}

vec3 random_cosine_direction();
vec3 random_cosine_direction(uint32_t seed);

vec3 random_to_sphere(float radius, float distance_squared);


vec3 reflect(const vec3& v, const vec3& n);
vec3 refract(const vec3& uv, const vec3& n, float etai_over_etat);
float reflectance(float cosine, float ref_idx);
bool flip_normal_if_front_face(const vec3& in_ray_direction, const vec3& in_outward_normal, vec3& out_normal);
inline float lerp_float(float a, float b, float f) { return a + f * (b - a); }
vec3 lerp_vec3(const vec3& a, const vec3& b, float f);
vec3 clamp_vec3(float a, float b, const vec3& f);
inline float min1(float a, float b) { return a < b ? a : b; }
inline float max1(float a, float b) { return a < b ? b : a; }
inline float clamp(float f, float a, float b) { return  min1(b, max1(a, f)); }
inline float smoothstep(float a, float b, float x)
{
  // https://thebookofshaders.com/glossary/?search=smoothstep
  float t = clamp(0.0f, 1.0f, (x - a) / (b - a));
  return t * t * (3.0f - 2.0f * t);
}
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
  vec3 pp = unit_vector(p); // normalize to get sensible values for acos, otherwise floating point exceptions will happen
  float theta = acos(-pp.y);
  float phi = atan2(-pp.z, pp.x) + pi;
  out_u = phi / (2.0f * pi);
  out_v = theta / pi;
}

namespace tone_mapping
{
  // Tone mapping functions
  // Based on https://64.github.io/tonemapping/
  // radiance vs luminance 
  // - radiance is physical measure (RGB), the amount of light incoming to a point from a single direction
  // - luminance is how humans perceive it, how bright we see something? (scalar) Human perception is non-linear.
  // HDR - High Dynamic Range
  // LDR - Low Dynamic Range
  // TMO - Tone Mapping Operator

  vec3 trivial(const vec3& v);
  vec3 reinhard(const vec3& v);
  vec3 reinhard_extended(const vec3& v, float max_white);
  float luminance(const vec3& v);
  vec3 change_luminance(const vec3& c_in, float l_out);
  vec3 reinhard_extended_luminance(const vec3& v, float max_white_l);
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

namespace paths
{
  // Directories
  std::string get_working_dir();
  std::string get_workspace_dir();
  std::string get_objects_dir();
  std::string get_images_dir();

  // Files
  std::string get_workspace_file_path(const char* file_name);
  std::string get_images_file_path(const char* file_name);
  std::string get_objects_file_path(const char* file_name);
  std::string get_window_file_path();
  std::string get_scene_file_path();
  std::string get_rendering_file_path();
  std::string get_imgui_file_path();
  std::string get_render_output_file_path();
} 

namespace obj_helper
{
  struct triangle_face
  {
    vec3 vertices[3];

    bool has_normals = false;
    vec3 normals[3];

    bool has_UVs = false;
    vec3 UVs[3]; //xy
  };

  bool load_obj(const std::string& file_name, int shape_index, std::vector<triangle_face>& out_faces);
}