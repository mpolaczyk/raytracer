#include "stdafx.h"

#include <filesystem>
#include <random>
#include <chrono>

#include "common.h"

float degrees_to_radians(float degrees)
{
  return degrees * pi / 180.0f;
}

float sign(float value)
{
  return value >= 0.0f ? 1.0f : -1.0f;  //  Assume 0 is positive
}

bool is_almost_zero(float value)
{
  return value <= epsilon && value >= -epsilon;
}

bool is_almost_equal(float a, float b)
{
  return fabs(a - b) <= epsilon;
}

vec3 random_in_unit_disk() 
{
  vec3 random_unit = unit_vector(random_cache::get_vec3());
  return random_unit * random_cache::get_float();
}

vec3 random_unit_in_hemisphere(const vec3& normal)
{
  vec3 random_unit = unit_vector(random_cache::get_vec3());
  // Invert random_unit if dot product is negative
  return sign(dot(random_unit, normal)) * random_unit;
}

vec3 random_in_unit_sphere()
{
  while (true) 
  {
    vec3 p = random_cache::get_vec3();
    if (p.length_squared() >= 1) continue;
    return p;
  }
}

vec3 random_cosine_direction()
{
  // Cosine distribution around positive z axis
  float r1 = random_cache::get_float_0_1();
  float r2 = random_cache::get_float_0_1();
  float phi = 2 * pi * r1;
  float x = cos(phi) * sqrt(r2);
  float y = sin(phi) * sqrt(r2);
  float z = sqrt(1 - r2);
  return vec3(x, y, z);
}

vec3 random_to_sphere(float radius, float distance_squared)
{
  float r1 = random_cache::get_float_0_1();
  float r2 = random_cache::get_float_0_1();
  float z = 1 + r2 * (sqrt(1 - radius * radius / distance_squared) - 1);

  float phi = 2 * pi * r1;
  float x = cos(phi) * sqrt(1 - z * z);
  float y = sin(phi) * sqrt(1 - z * z);

  return vec3(x, y, z);
}

bool is_near_zero(vec3& value)
{
  return (fabs(value[0]) < small_number) && (fabs(value[1]) < small_number) && (fabs(value[2]) < small_number);
}

vec3 reflect(const vec3& vec, const vec3& normal) 
{
  return vec - 2 * dot(vec, normal) * normal;
}

vec3 refract(const vec3& v, const vec3& n, float etai_over_etat) 
{
  float cos_theta = fmin(dot(-v, n), 1.0f);
  vec3 r_out_perpendicular = etai_over_etat * (v + cos_theta * n);
  vec3 r_out_parallel = -sqrt(fabs(1.0f - r_out_perpendicular.length_squared())) * n;
  return r_out_perpendicular + r_out_parallel;
}

float reflectance(float cosine, float ref_idx)
{
  // Use Schlick's approximation for reflectance.
  float r0 = (1.0f - ref_idx) / (1.0f + ref_idx);
  r0 = r0 * r0;
  return r0 + (1.0f - r0) * pow((1.0f - cosine), 5.0f);
}

bool flip_normal_if_front_face(const vec3& in_ray_direction, const vec3& in_outward_normal, vec3& out_normal)
{
  if (dot(in_ray_direction, in_outward_normal) < 0)
  {
    // Ray is inside
    out_normal = in_outward_normal;
    return true;
  }
  else
  {
    // Ray is outside
    out_normal = -in_outward_normal;
    return false;
  }
}

vec3 lerp_vec3(vec3 a, vec3 b, float f)
{
  return vec3(lerp_float(a.x, b.x, f), lerp_float(a.y, b.y, f), lerp_float(a.z, b.z, f));
}

namespace random_cache
{
  template<typename T, int N>
  void cache_internal<T,N>::init(float min, float max)
  {
    distribution = std::uniform_real_distribution<T>(min, max);
    uint32_t seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 generator(seed);
    for (int s = 0; s < num; s++)
    {
      cache.push_back(distribution(generator));
    }
  }

  template<typename T, int N>
  float cache_internal<T, N>::get()
  {
    last_index = (last_index + 1) % num;
    return cache[last_index];
  }

  void init()
  {
    f_cache.init(-1.0f, 1.0f);
  }

  float get_float()
  {
    return f_cache.get();
  }

  vec3 get_vec3()
  {
    return vec3(f_cache.get(), f_cache.get(), f_cache.get());
  }

  float get_float_0_1()
  {
    return fabs(f_cache.get());
  }

  float get_float_0_N(float N)
  {
    return fabs(f_cache.get()) * N;
  }

  float get_float_M_N(float M, float N)
  {
    if (M < N)
    {
      return M + fabs(f_cache.get()) * (N - M);
    }
    return N + fabs(f_cache.get()) * (M-N);
  }

  vec3 get_vec3_0_1()
  {
    return vec3(fabs(f_cache.get()), fabs(f_cache.get()), fabs(f_cache.get()));
  }

  int32_t get_int_0_N(int32_t N)
  {
    return round(get_float_0_1() * N);
  }
}

namespace paths
{
  std::string get_working_dir()
  {
    std::string current_dir = std::filesystem::current_path().string();
    std::ostringstream oss;
    oss << current_dir << "\\";
    return oss.str();
  }

  std::string get_project_dir()
  {
    std::string working_dir = get_working_dir();
    std::ostringstream oss;
    oss << working_dir << "..\\..\\Project\\";
    return oss.str();
  }

  std::string get_project_file_path(const char* project_file_name)
  {
    std::string project_dir = get_project_dir();
    std::ostringstream oss;
    oss << project_dir << project_file_name;
    return oss.str();
  }

  std::string get_window_file_path()
  {
    return get_project_file_path("window.json");
  }

  std::string get_scene_file_path()
  {
    return get_project_file_path("scene.json");
  }

  std::string get_rendering_file_path()
  {
    return get_project_file_path("rendering.json");
  }

  std::string get_render_output_file_path()
  {
    std::time_t result = std::time(nullptr);
    std::ostringstream oss;
    oss << "output_" << result << ".bmp";
    return get_project_file_path(oss.str().c_str());
  }

  std::string get_imgui_file_path()
  {
    return get_project_file_path("imgui.ini");
  }
}