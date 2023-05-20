#include "stdafx.h"

#include <ppl.h>
#include <math.h>

#include "math/materials.h"

#include "reference_renderer.h"

std::string reference_renderer::get_name() const
{
  return "Reference CPU";
}

void reference_renderer::render()
{
  save_output = true;

  std::vector<chunk> chunks;
  const int chunks_per_thread = 32;
  chunk_generator::generate_chunks(chunk_strategy_type::rectangles, std::thread::hardware_concurrency() * chunks_per_thread, job_state.image_width, job_state.image_height, chunks);

  concurrency::parallel_for_each(begin(chunks), end(chunks), [&](chunk ch) { render_chunk(ch); });
}


void reference_renderer::render_chunk(const chunk& in_chunk)
{
  std::thread::id thread_id = std::this_thread::get_id();

  std::ostringstream oss;
  oss << "Thread=" << thread_id << " Chunk=" << in_chunk.id;
  const char* name = oss.str().c_str();
  benchmark::scope_counter benchmark_render_chunk(name, false);

  vec3 resolution(job_state.image_width, job_state.image_height, 0.0f);

  for (int y = in_chunk.y; y < in_chunk.y + in_chunk.size_y; ++y)
  {
    for (int x = in_chunk.x; x < in_chunk.x + in_chunk.size_x; ++x)
    {
      vec3 hdr_color = fragment(x, y, resolution);

      assert(isfinite(hdr_color.x));
      assert(isfinite(hdr_color.y));
      assert(isfinite(hdr_color.z));

      vec3 ldr_color = tone_mapping::reinhard_extended(hdr_color, job_state.renderer_conf.white_point);
      
      bmp::bmp_pixel p(ldr_color);
      job_state.img_rgb->draw_pixel(x, y, &p, bmp::bmp_format::rgba);
      if (save_output)
      {
        job_state.img_bgr->draw_pixel(x, y, &p);      
      }
    }
  }
}

vec3 reference_renderer::fragment(float x, float y, const vec3& resolution)
{
  uint32_t seed = uint32_t(y * resolution.x + x);  // Each pixel has a unique seed, gradient from white to black

  const int rays_per_pixel = job_state.renderer_conf.rays_per_pixel;
  vec3 sum_colors;

  for (int i = 0; i < rays_per_pixel; ++i)
  {
    // Anti aliasing with ray variance
    float u = (float(x) + rand_pcg(seed)) / (resolution.x - 1);
    float v = (float(y) + rand_pcg(seed)) / (resolution.y - 1);
    // Trace the ray
    ray r = job_state.cam.get_ray(u, v);
    sum_colors += trace_ray(r, seed);
  }
  
  vec3 hdr_color = sum_colors / (float)rays_per_pixel;

  return hdr_color;
}

vec3 reference_renderer::enviroment_light(const ray& in_ray)
{
  static const vec3 sky_color_zenith = c_white_blue;
  static const vec3 sky_color_horizon = c_white;
  static const float sky_brightness = 0.4f;

  float t = smoothstep(-0.6f, 0.2f, in_ray.direction.y);
  vec3 light = lerp_vec3(sky_color_horizon, sky_color_zenith, t);
  return clamp_vec3(0.0f, 1.0f, light) * sky_brightness;
}

vec3 reference_renderer::trace_ray(ray in_ray, uint32_t seed)
{
  // Defined by material color of all bounces, mixing colors (multiply to aggregate) [0.0f, 1.0f]
  vec3 ray_color = vec3(1.0f);

  // Defined by emitted color of all emissive bounces weighted by the material color (add to aggregate) [0.0f, inf] 
  // Allow light to exceed 1.0f. Non-emissive materials can emit a little bit of light
  vec3 incoming_light = vec3(0.0f);

  int bounces = job_state.renderer_conf.ray_bounces;
  for (int i = 0; i < bounces; ++i)
  {
    hit_record hit;
    if (job_state.scene_root.hit(in_ray, 0.01f, infinity, hit))
    {
      // Don't bounce if ray has no color
      if (ray_color.length_squared() < 0.1f)
      {
        break;
      }

      // Read material
      material mat = *hit.material_ptr;

      if (mat.type == material_type::light)
      {
        // Don't bounce of lights
        incoming_light += mat.emitted_color * ray_color;
        break;
      }

      in_ray.origin = hit.p;

      bool can_gloss_bounce = mat.gloss_probability >= rand_pcg(seed);
      bool can_refract = mat.refraction_probability >= rand_pcg(seed);
      
      vec3 diffuse_dir = normalize(hit.normal + rand_direction(seed));

      if (!can_gloss_bounce && can_refract)
      {
        float refraction_ratio = hit.front_face ? (1.0f / mat.refraction_index) : mat.refraction_index;
        vec3 refraction_dir = refract(in_ray.direction, hit.normal, refraction_ratio);

        // Define next bounce
        in_ray.direction = refraction_dir + diffuse_dir*(1.0f-mat.smoothness);
        
        // Refraction color
        ray_color *= mat.color;

        assert(ray_color.is_valid_color());
        continue;
      }

      // New directions
      
      vec3 specular_dir = reflect(in_ray.direction, hit.normal);

      // Define next bounce
      float blend = mat.smoothness;
      if (mat.gloss_probability > 0.0f)
      {
        blend = mat.smoothness * can_gloss_bounce;
      }
      in_ray.direction = lerp_vec3(diffuse_dir, specular_dir, blend);

      // Calculate color for this hit
      incoming_light += mat.emitted_color * ray_color;
      ray_color *= lerp_vec3(mat.color, mat.gloss_color, can_gloss_bounce);
      assert(ray_color.is_valid_color());
    }
    else
    {
      vec3 env_light = enviroment_light(in_ray);  // environment is emissive
      incoming_light += env_light * ray_color;
      break;
    }
  }

  return incoming_light;
}