#include "stdafx.h"

#include <filesystem>
#include <random>

#include "gfx/tiny_obj_loader.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include "common.h"

namespace math
{
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

  vec3 reflect(const vec3& vec, const vec3& normal)
  {
    return vec - 2 * dot(vec, normal) * normal;
  }

  vec3 refract(const vec3& v, const vec3& n, float etai_over_etat)
  {
    float cos_theta = fmin(dot(-v, n), 1.0f);
    vec3 r_out_perpendicular = etai_over_etat * (v + cos_theta * n);
    vec3 r_out_parallel = -sqrt(fabs(1.0f - math::length_squared(r_out_perpendicular))) * n;
    return r_out_perpendicular + r_out_parallel;
  }

  vec3 lerp_vec3(const vec3& a, const vec3& b, float f)
  {
    return vec3(math::lerp_float(a.x, b.x, f), math::lerp_float(a.y, b.y, f), math::lerp_float(a.z, b.z, f));
  }

  vec3 clamp_vec3(float a, float b, const vec3& f)
  {
    vec3 ans;
    ans.x = math::clamp(f.x, a, b);
    ans.y = math::clamp(f.y, a, b);
    ans.z = math::clamp(f.z, a, b);
    return ans;
  }

  bool ray_triangle(const ray& in_ray, float t_min, float t_max, const triangle_face* in_triangle, hit_record& out_hit)
  {
    assert(in_triangle != nullptr);
    using namespace math;

    // https://graphicscodex.courses.nvidia.com/app.html?page=_rn_rayCst "4. Ray-Triangle Intersection"
    
    // Vertices
    const __m128 V0 = in_triangle->vertices[0].R128;
    const __m128 V1 = in_triangle->vertices[1].R128;
    const __m128 V2 = in_triangle->vertices[2].R128;

    // Ray origin and direction
    const __m128 P = in_ray.origin.R128;
    const __m128 w = in_ray.direction.R128;
      
    // Edge vectors
    const __m128 E1 = _mm_sub_ps(V1, V0);
    const __m128 E2 = _mm_sub_ps(V2, V0);

    // Face normal
    __m128 n = vnormalize(vcross(E1, E2));

    // Plane intersection what is q and a?
    const __m128 q = vcross(w, E2);
    const float a = vdot(E1, q);
      
    // Ray parallel or close to the limit of precision?
    if (fabsf(a) <= small_number) [[unlikely]] return false;

    // ?
    const __m128& s = _mm_div_ps(_mm_sub_ps(P, V0), _mm_set1_ps(a));
    const __m128& r = vcross(s, E1);

    // Barycentric coordinates
    float b[3];
    b[0] = vdot(s, q);
    b[1] = vdot(r, w);
    b[2] = 1.0f - (b[0] + b[1]);
      
    // Intersection outside of triangle?
    if ((b[0] < 0.0f) || (b[1] < 0.0f) || (b[2] < 0.0f)) [[likely]] return false;
      
    // Distance to intersection
    const float t = vdot(E2, r);

    // Intersection outside of ray range?
    if (t < t_min || t > t_max) [[unlikely]] return false;

    // Detect backface
    // !!!! it works but should be the opposite! are faces left or right oriented?
    const float dot_n_w = vdot(n, w);
    n = _mm_mul_ps(n, _mm_set1_ps(-1.0f * sign(dot_n_w)));
      
    const vec3 barycentric(b[2], b[0], b[1]);
    // this does not work, TODO translate normals?
    //out_hit.normal = normalize(barycentric[0] * in_triangle->normals[0] + barycentric[1] * in_triangle->normals[1] + barycentric[2] * in_triangle->normals[2]);
    const vec3 uv = barycentric[0] * in_triangle->UVs[0] + barycentric[1] * in_triangle->UVs[1] + barycentric[2] * in_triangle->UVs[2];

    out_hit.p = in_ray.at(t);
    out_hit.normal = vec3(n);
    out_hit.t = t;
    out_hit.u = uv.x;
    out_hit.v = uv.y;
    out_hit.front_face = dot_n_w < 0.0f;

    return true;
  }
}

namespace random_seed
{
  float rand_iqint1(uint32_t seed)
  {
    static uint32_t last = 0;
    uint32_t state = seed + last;   // Seed can be the same for multiple calls, we need to rotate it
    state = (state << 13U) ^ state;
    state = state * (state * state * 15731U + 789221U) + 1376312589U;
    last = state;
    return (float)state / (float)UINT_MAX;   // [0.0f, 1.0f]
  }

  float rand_pcg(uint32_t seed)
  {
    static uint32_t last = 0;
    uint32_t state = seed + last;   // Seed can be the same for multiple calls, we need to rotate it
    state = state * 747796405U + 2891336453U;
    uint32_t word = ((state >> ((state >> 28U) + 4U)) ^ state) * 277803737U;
    uint32_t result = ((word >> 22U) ^ word);
    last = result;
    return (float)result / (float)UINT_MAX;   // [0.0f, 1.0f]
  }

  vec3 direction(uint32_t seed)
  {
    float x = normal_distribution(seed);
    float y = normal_distribution(seed);
    float z = normal_distribution(seed);
    return math::normalize(vec3(x, y, z));
  }
  
  float normal_distribution(uint32_t seed)
  {
    float theta = 2.0f * math::pi * RAND_SEED_FUNC(seed);
    float rho = sqrt(-2.0f * log(RAND_SEED_FUNC(seed)));
    assert(isfinite(rho));
    return rho * cos(theta);  // [-1.0f, 1.0f]
  }

  vec3 cosine_direction(uint32_t seed)
  {
    // https://psgraphics.blogspot.com/2013/11/random-directions-with-cosine.html
    // Cosine distribution around positive z axis
    float r1 = RAND_SEED_FUNC(seed);
    float r2 = RAND_SEED_FUNC(seed);
    //float phi = 2 * pi * r1;
    float r = 0.5;
    float x = cos(2 * math::pi * r1) * sqrt(r);
    float y = sin(2 * math::pi * r2) * sqrt(r);
    float z = sqrt(1 - r);
    return vec3(x, y, z);
  }
}

namespace random_cache
{
  template<typename T, int N>
  T cache<T, N>::get()
  {
    assert(last_index >= 0 && last_index < N);
    last_index = (last_index + 1) % N;
    return storage[last_index];
  }

  template<typename T, int N>
  void cache<T, N>::add(T value)
  {
    storage.push_back(value);
  }

  template<typename T, int N>
  int cache<T, N>::len()
  {
    return N;
  }

  void init()
  {
    // Fill float cache
    std::uniform_real_distribution<float> distribution;
    distribution = std::uniform_real_distribution<float>(-1.0f, 1.0f);
    std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());
    for (int s = 0; s < float_cache.len(); s++)
    {
      float_cache.add(distribution(generator));
    }

    // Fill cosine direction cache
    for (int s = 0; s < cosine_direction_cache.len(); s++)
    {
      cosine_direction_cache.add(cosine_direction());
    } 
  }

  float get_float()
  {
    return float_cache.get();
  }

  float get_float_0_1()
  {
    return fabs(float_cache.get());
  }

  float get_float_0_N(float N)
  {
    return fabs(float_cache.get()) * N;
  }

  float get_float_M_N(float M, float N)
  {
    if (M < N)
    {
      return M + fabs(float_cache.get()) * (N - M);
    }
    return N + fabs(float_cache.get()) * (M - N);
  }

  vec3 get_vec3() // [-1,1]
  {
    return vec3(float_cache.get(), float_cache.get(), float_cache.get());
  }

  vec3 get_vec3_0_1()
  {
    return vec3(fabs(float_cache.get()), fabs(float_cache.get()), fabs(float_cache.get()));
  }

  int get_int_0_N(int N)
  {
    return (int)round(get_float_0_1() * N);
  }

  vec3 get_cosine_direction()
  {
    return cosine_direction_cache.get();
  }

  vec3 direction()
  {
    float x = normal_distribution();
    float y = normal_distribution();
    float z = normal_distribution();
    return math::normalize(vec3(x, y, z));
  }

  vec3 cosine_direction()
  {
    // https://psgraphics.blogspot.com/2013/11/random-directions-with-cosine.html
    // Cosine distribution around positive z axis
    float r1 = get_float_0_1();
    float r2 = get_float_0_1();
    //float phi = 2 * pi * r1;
    float r = 0.5;
    float x = cos(2 * math::pi * r1) * sqrt(r);
    float y = sin(2 * math::pi * r2) * sqrt(r);
    float z = sqrt(1 - r);
    return vec3(x, y, z);
  }

  vec3 in_sphere(float radius, float distance_squared)
  {
    float r1 = get_float_0_1();
    float r2 = get_float_0_1();
    float z = 1 + r2 * (sqrt(1 - radius * radius / distance_squared) - 1);

    float phi = 2 * math::pi * r1;
    float x = cos(phi) * sqrt(1 - z * z);
    float y = sin(phi) * sqrt(1 - z * z);

    return vec3(x, y, z);
  }

  float normal_distribution()
  {
    float theta = 2.0f * math::pi * get_float_0_1();
    float rho = sqrt(-2.0f * log(get_float_0_1()));
    assert(isfinite(rho));
    return rho * cos(theta);  // [-1.0f, 1.0f]
  }
}

namespace tone_mapping
{
  vec3 trivial(const vec3& v)
  {
    return math::clamp_vec3(0.0f, 1.0f, v);
  }
  vec3 reinhard(const vec3& v)
  {
    // Mathematically guarantees to produce [0.0, 1.0]
    // Use with luminance not with RGB radiance
    return v / (1.0f + v);
  }
  vec3 reinhard_extended(const vec3& v, float max_white)
  {
    vec3 numerator = v * (1.0f + (v / vec3(max_white * max_white)));
    return numerator / (1.0f + v);
  }
  float luminance(const vec3& v)
  {
    float value = math::dot(v, vec3(0.2126f, 0.7152f, 0.0722f));
    assert(value >= 0.0f);
    return value;
  }
  vec3 change_luminance(const vec3& c_in, float l_out)
  {
    float l_in = luminance(c_in);
    if (l_in == 0.0f)
    {
      return vec3(0, 0, 0);
    }
    return c_in * (l_out / l_in);
  }
  vec3 reinhard_extended_luminance(const vec3& v, float max_white_l)
  {
    assert(max_white_l > 0.0f);
    float l_old = luminance(v);
    float numerator = l_old * (1.0f + (l_old / (max_white_l * max_white_l)));
    float l_new = numerator / (1.0f + l_old);
    return change_luminance(v, l_new);
  }
}

namespace hash
{
  uint32_t combine(uint32_t a, uint32_t b)
  {
    uint32_t c = 0x9e3779b9;
    a += c;

    a -= c; a -= b; a ^= (b >> 13);
    c -= b; c -= a; c ^= (a << 8);
    b -= a; b -= c; b ^= (c >> 13);
    a -= c; a -= b; a ^= (b >> 12);
    c -= b; c -= a; c ^= (a << 16);
    b -= a; b -= c; b ^= (c >> 5);
    a -= c; a -= b; a ^= (b >> 3);
    c -= b; c -= a; c ^= (a << 10);
    b -= a; b -= c; b ^= (c >> 15);

    return b;
  }
  uint32_t combine(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
  {
    return combine(combine(a, b), combine(c, d));
  }
  uint32_t get(uint64_t a)
  {
    return (uint32_t)a + ((uint32_t)(a >> 32) * 23);
  }
  uint32_t get(int64_t a)
  {
    return (uint32_t)a + ((uint32_t)(a >> 32) * 23);
  }
  uint32_t get(float a)
  {
    return reinterpret_cast<uint32_t&>(a);
  }
  uint32_t get(double a)
  {
    return get(reinterpret_cast<uint64_t&>(a));
  }
  uint32_t get(const void* a)
  {
    return get(reinterpret_cast<size_t>(a));
  }
  uint32_t get(void* a)
  {
    return get(reinterpret_cast<size_t>(a));
  }
  uint32_t get(bool a)
  {
    return (uint32_t)a;
  }
  uint32_t get(const vec3& a)
  {
    return combine(get(a.x), get(a.y), get(a.z), get(a.padding));
  }
}

namespace io
{
  std::string get_working_dir()
  {
    std::string current_dir = std::filesystem::current_path().string();
    std::ostringstream oss;
    oss << current_dir << "\\";
    return oss.str();
  }

  std::string get_workspace_dir()
  {
    std::string working_dir = get_working_dir();
    std::ostringstream oss;
    oss << working_dir << "..\\..\\Workspace\\";
    return oss.str();
  }

  std::string get_objects_dir()
  {
    std::string workspace_dir = get_workspace_dir();
    std::ostringstream oss;
    oss << workspace_dir << "Objects\\";
    return oss.str();
  }

  std::string get_images_dir()
  {
    std::string workspace_dir = get_workspace_dir();
    std::ostringstream oss;
    oss << workspace_dir << "Images\\";
    return oss.str();
  }


  std::string get_workspace_file_path(const char* file_name)
  {
    std::string workspace_dir = get_workspace_dir();
    std::ostringstream oss;
    oss << workspace_dir << file_name;
    return oss.str();
  }

  std::string get_images_file_path(const char* file_name)
  {
    std::string images_dir = get_images_dir();
    std::ostringstream oss;
    oss << images_dir << file_name;
    return oss.str();
  }

  std::string get_objects_file_path(const char* file_name)
  {
    std::string objects_dir = get_objects_dir();
    std::ostringstream oss;
    oss << objects_dir << file_name;
    return oss.str();
  }

  std::string get_window_file_path()
  {
    return get_workspace_file_path("window.json");
  }

  std::string get_scene_file_path()
  {
    return get_workspace_file_path("scene.json");
  }

  std::string get_rendering_file_path()
  {
    return get_workspace_file_path("rendering.json");
  }

  std::string get_imgui_file_path()
  {
    return get_workspace_file_path("imgui.ini");
  }

  std::string get_render_output_file_path()
  {
    std::time_t t = std::time(nullptr);
    std::ostringstream oss;
    oss << "output_" << t << ".bmp";
    return get_images_file_path(oss.str().c_str());
  }

}

namespace obj_helper
{
  bool load_obj(const std::string& file_name, int shape_index, std::vector<triangle_face>& out_faces)
  {
    assert(shape_index >= 0);

    tinyobj::attrib_t attributes; // not implemented
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials; // not implemented

    std::string dir = io::get_objects_dir();
    std::string path = io::get_objects_file_path(file_name.c_str());

    std::string error;
    if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &error, path.c_str(), dir.c_str(), true))
    {
      logger::error("Unable to load object file: {0}", file_name);
      return false;
    }
    if (shape_index >= shapes.size())
    {
      logger::error("Object file: {0} does not have shape index: {1}", file_name, shape_index);
      return false;
    }

    tinyobj::shape_t shape = shapes[shape_index];
    size_t num_faces = shape.mesh.num_face_vertices.size();
    if (num_faces == 0)
    {
      logger::error("Object file: {0} has no faces", file_name);
      return false;
    }
    out_faces.reserve(num_faces);

    // loop over faces
    for (size_t fi = 0; fi < num_faces; ++fi)
    {
      out_faces.push_back(triangle_face());
      triangle_face& face = out_faces[fi];

      // loop over the vertices in the face
      assert(shape.mesh.num_face_vertices[fi] == 3);
      for (size_t vi = 0; vi < 3; ++vi)
      {
        tinyobj::index_t idx = shape.mesh.indices[3 * fi + vi];

        if (idx.vertex_index == -1)
        {
          logger::error("Object file: {0} faces not found", file_name);
          return false;
        }
        if (idx.normal_index == -1)
        {
          logger::error("Object file: {0} normals not found", file_name);
          return false;
        }
        if (idx.texcoord_index == -1)
        {
          logger::error("Object file: {0} UVs not found", file_name);
          return false;
        }

        float vx = attributes.vertices[3 * idx.vertex_index + 0];
        float vy = attributes.vertices[3 * idx.vertex_index + 1];
        float vz = attributes.vertices[3 * idx.vertex_index + 2];
        face.vertices[vi] = vec3(vx, vy, vz);

        float nx = attributes.normals[3 * idx.normal_index + 1];
        float ny = attributes.normals[3 * idx.normal_index + 2];
        float nz = attributes.normals[3 * idx.normal_index + 0];
        face.normals[vi] = vec3(nx, ny, nz);
        
        float uvx = attributes.texcoords[2 * idx.texcoord_index + 0];
        float uvy = attributes.texcoords[2 * idx.texcoord_index + 1];
        face.UVs[vi] = vec3(uvx, uvy, 0.0f);
      }
    }
    return true;
  }
}

namespace logger
{
  static auto console = spdlog::stdout_color_mt("console");

  void init()
  {
    spdlog::flush_every(std::chrono::seconds(3));
#if BUILD_DEBUG
    spdlog::set_level(spdlog::level::trace);
#elif BUILD_RELEASE
    spdlog::set_level(spdlog::level::info);
#endif
    spdlog::set_pattern("[%H:%M:%S.%e] [thread %t] [%l] %^%v%$");
  }
}