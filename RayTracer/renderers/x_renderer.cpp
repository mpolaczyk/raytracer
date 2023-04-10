#include "stdafx.h"

#include <ppl.h>
#include <math.h>

#include "math/materials.h"

#include "x_renderer.h"

std::string x_renderer::get_name() const
{
  return "Example";
}

void x_renderer::render()
{
  save_output = true;

  std::vector<chunk> chunks;
  const int chunks_per_thread = 32;
  chunk_generator::generate_chunks(chunk_strategy_type::vertical_stripes, std::thread::hardware_concurrency() * chunks_per_thread, ajs.image_width, ajs.image_height, chunks);

  concurrency::parallel_for_each(begin(chunks), end(chunks), [&](chunk ch) { render_chunk(ch); });
}


void x_renderer::render_chunk(const chunk& in_chunk)
{
  std::thread::id thread_id = std::this_thread::get_id();

  std::ostringstream oss;
  oss << "Thread=" << thread_id << " Chunk=" << in_chunk.id;
  const char* name = oss.str().c_str();
  benchmark::scope_counter benchmark_render_chunk(name, false);

  int w = ajs.image_width;
  int h = ajs.image_height;
  vec3 res(w, h, 0.0f);

  for (int y = in_chunk.y; y < in_chunk.y + in_chunk.size_y; ++y)
  {
    for (int x = in_chunk.x; x < in_chunk.x + in_chunk.size_x; ++x)
    {
      float u = float(x) / (w - 1);
      float v = float(y) / (h - 1);
      vec3 pixel_coord = vec3(u, v, 0.0f) * res;
      uint32_t seed = uint32_t(pixel_coord.y * res.x + pixel_coord.x);

      vec3 pixel_color = fragment(u, v, seed);

      assert(!isnan(pixel_color.x));
      assert(!isnan(pixel_color.y));
      assert(!isnan(pixel_color.z));
      if (isnan(pixel_color.x)) pixel_color.x = 0.0f;
      if (isnan(pixel_color.y)) pixel_color.y = 0.0f;
      if (isnan(pixel_color.z)) pixel_color.z = 0.0f;

      bmp::bmp_pixel p(pixel_color);
      ajs.img_rgb->draw_pixel(x, y, &p, bmp::bmp_format::rgba);
      if (save_output)
      {
        ajs.img_bgr->draw_pixel(x, y, &p);
        
      }
    }
  }
}

vec3 x_renderer::fragment(float u, float v, uint32_t seed)
{
  ray r = ajs.cam.get_ray(u, v);

  const int rays_per_pixel = 600;
  vec3 pixel_color;
  for (int i = 0; i < rays_per_pixel; ++i)
  {
    pixel_color += ray_color(r, seed);
  }
  
  return pixel_color / (float)rays_per_pixel;
}

vec3 x_renderer::enviroment_light(const ray& in_ray)
{
  static const vec3 sky_color_zenith = c_white_blue;
  static const vec3 sky_color_horizon = c_white;
  
  float t = smoothstep(-0.4f, 0.2f, in_ray.direction.y);
  return lerp_vec3(sky_color_horizon, sky_color_zenith, t);
}

vec3 x_renderer::ray_color(ray in_ray, uint32_t seed)
{
  vec3 incoming_light = vec3(0.0f);
  vec3 color = vec3(1.0f);

  const int max_bounces = 4;
  for (int i = 0; i < max_bounces; ++i)
  {
    hit_record hit;
    if (ajs.scene_root.hit(in_ray, 0.001f, infinity, hit))  // potential work to save, first hit always the same
    {
      // Define next bounce
      in_ray.origin = hit.p;
      //in_ray.direction = random_unit_in_hemisphere(hit.normal, seed);
      in_ray.direction = normalize(hit.normal + rand_direction(seed));

      // Calculate color for this bounce
      vec3 emitted_light = hit.material_ptr->emitted(hit);
      //float light_strength = dot(hit.normal, in_ray.direction);
      incoming_light += emitted_light * color;
      //color *= hit.material_ptr->color() * light_strength * 2.0f;
      color *= hit.material_ptr->color();
      
    }
    else
    {
      incoming_light += enviroment_light(in_ray) * color;
      break;
    }
  }

  return incoming_light;
}