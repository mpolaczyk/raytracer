#include "stdafx.h"

#include <vector>
#include <ppl.h>

#include "rtow_renderer.h"
#include "thread_pool.h"

#include "materials.h"

std::string rtow_renderer::get_name() const
{
  return "RT in One Weekend";
}

void rtow_renderer::render()
{
  // Build chunks of work
  std::vector<chunk> chunks;
  chunk_generator::generate_chunks(ajs.settings.chunks_strategy, ajs.settings.chunks_num, ajs.image_width, ajs.image_height, chunks);

  if (ajs.settings.shuffle_chunks)
  {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(chunks.begin(), chunks.end(), g);
  }

  // Process chunks on parallel
  if (ajs.settings.threading_strategy == threading_strategy_type::none)
  {
    chunk ch;
    ch.id = 1;
    ch.size_x = ajs.image_width;
    ch.size_y = ajs.image_height;
    render_chunk(ch);
  }
  if (ajs.settings.threading_strategy == threading_strategy_type::thread_pool)
  {
    thread_pool pool;
    for (const chunk& ch : chunks)
    {
      pool.queue_job([&]() { render_chunk(ch); });
    }
    pool.start(ajs.settings.threads_num > 0 ? ajs.settings.threads_num : std::thread::hardware_concurrency());
    while (pool.is_busy()) {}
    pool.stop();
  }
  else if (ajs.settings.threading_strategy == threading_strategy_type::pll_for_each)
  {
    concurrency::parallel_for_each(begin(chunks), end(chunks), [&](chunk ch) { render_chunk(ch); });
  }
}

void rtow_renderer::render_chunk(const chunk& in_chunk)
{
  std::thread::id thread_id = std::this_thread::get_id();

  std::ostringstream oss;
  oss << "Thread=" << thread_id << " Chunk=" << in_chunk.id;
  const char* name = oss.str().c_str();
  benchmark::scope_counter benchmark_render_chunk(name, false);

  for (int y = in_chunk.y; y < in_chunk.y + in_chunk.size_y; ++y)
  {
    for (int x = in_chunk.x; x < in_chunk.x + in_chunk.size_x; ++x)
    {
      benchmark::instance timer;
      if (ajs.settings.pixel_time_coloring)
      {
        timer.start("RayColor", false);
      }
      // Anti Aliasing done at the ray level, multiple rays for each pixel.
      vec3 pixel_color;
      for (int c = 0; c < ajs.settings.AA_samples_per_pixel; c++)
      {
        float u = (float(x) + random_cache::get_float()) / (ajs.image_width - 1);
        float v = (float(y) + random_cache::get_float()) / (ajs.image_height - 1);
        ray r = ajs.cam.get_ray(u, v);
        vec3 sample = ray_color(r, ajs.settings.diffuse_max_bounce_num);
        assert(!isnan(sample.x));
        assert(!isnan(sample.y));
        assert(!isnan(sample.z));
        if (isnan(sample.x)) sample.x = 0.0f;
        if (isnan(sample.y)) sample.y = 0.0f;
        if (isnan(sample.z)) sample.z = 0.0f;
        pixel_color += sample;
        // TODO: measure variance, don't send too much rays when variance is low
      }
      // Save to bmp
      bmp::bmp_pixel p;
      if (ajs.settings.pixel_time_coloring)
      {
        uint64_t t = timer.stop();
        p = bmp::bmp_pixel(t / (float)ajs.settings.AA_samples_per_pixel * ajs.settings.pixel_time_coloring_scale);
      }
      else
      {
        p = bmp::bmp_pixel(pixel_color / (float)ajs.settings.AA_samples_per_pixel);
      }
      ajs.img_bgr->draw_pixel(x, y, &p);
      ajs.img_rgb->draw_pixel(x, y, &p, bmp::bmp_format::rgba);
    }
  }
}

vec3 inline rtow_renderer::ray_color(const ray& in_ray, uint32_t depth)
{
  if (depth <= 0)
  {
    return c_black;
  }

  hit_record hit;
  if (!ajs.scene_root.hit(in_ray, 0.001f, infinity, hit))
  {
    return c_white;
  }

  vec3 c_emissive = hit.material_ptr->emitted(hit);

  scatter_record sr;
  if (!hit.material_ptr->scatter(in_ray, hit, sr))
  {
    return c_emissive;
  }
  else if (sr.is_specular)
  {
    vec3 c_specular = sr.attenuation * ray_color(sr.specular_ray, depth - 1);
    return c_specular;
  }
  else if (sr.is_diffuse)
  {
    int32_t option = -2;

    if (option == -2) // FINAL
    {
      // book 3, The Mixture PDF Class
      hittable* light = ajs.scene_root.get_random_light();

      // Warning!
      // Just light pdf can produce good results with low number of rays (low noise in light, small noise in shadow).
      //   but using only it generate image with lots of very dark shadows and gives too much reflection close to the light.
      // Just material pdf can produce good results only with high number of rays (noise everywhere)(order of magnitude more rays)
      //   but it wastes a lot of rays hitting elements that are not light - no material addition to the final color
      // That's why we have to blend both.
      // Q: Can we render two passes and then blend?
      //  - light with low count
      //  - material with higher
      // Q: Can we denoise only material output?
      hittable_pdf light_pdf(light, hit.p);
      cosine_pdf material_pdf = cosine_pdf(hit.normal);

      // I'm not satisfied with results. I can barely achieve results presented on the pictures, book 3 seems to have so many bugs.
      // Originally book uses mixture_pdf, it uses direction vectors in turns and mixes values, I'd try different approach here too.
      // Light pdf produce low noise but super dark image.
      // Material pdf is super noisy and also is brighter with more lights which decrease the quality after mixing.
      // I'm going to move to DXR and DX12 as I'm more interested in the pipeline than math at this point.
      vec3 pdf_direction;
      float pdf_val;
      if (ajs.settings.pdf_mix_type)
      {
        mixture_pdf mix_pdf = mixture_pdf(&light_pdf, &material_pdf, ajs.settings.pdf_ratio);
        pdf_direction = mix_pdf.get_direction();
        pdf_val = mix_pdf.get_value(pdf_direction);
      }
      else
      {
        if (is_almost_equal(0.0f, ajs.settings.pdf_ratio) || random_cache::get_float_0_1() > ajs.settings.pdf_ratio)
        {
          pdf_direction = light_pdf.get_direction();
          pdf_val = light_pdf.get_value(pdf_direction);
        }
        else if (is_almost_equal(1.0f, ajs.settings.pdf_ratio) || random_cache::get_float_0_1() < ajs.settings.pdf_ratio)
        {
          pdf_direction = material_pdf.get_direction();
          pdf_val = material_pdf.get_value(pdf_direction);
        }
      }

      ray scattered = ray(hit.p, unit_vector(pdf_direction));
      float scattering_pdf = hit.material_ptr->scatter_pdf(in_ray, hit, scattered);

      vec3 c_diffuse = sr.attenuation * scattering_pdf * ray_color(scattered, depth - 1) / pdf_val;

      return c_emissive + c_diffuse;
    }
    else if (option == -1)
    {
      // book 3, chapter 10.1 The PDF class  (light pdf only)
      hittable* light = ajs.scene_root.get_random_light();
      hittable_pdf light_pdf(light, hit.p);

      vec3 to_light = light_pdf.get_direction();
      float pdf_val = light_pdf.get_value(to_light);

      ray scattered = ray(hit.p, to_light);
      float scattering_pdf = hit.material_ptr->scatter_pdf(in_ray, hit, scattered);

      vec3 c_diffuse = (sr.attenuation * scattering_pdf * ray_color(scattered, depth - 1)) / pdf_val;

      return c_emissive + c_diffuse;
    }
    else if (option == 0)
    {
      // book 3, chapter 9.2 Light Sampling
      hittable* light = ajs.scene_root.get_random_light();

      // get_pdf_direction
      vec3 on_light = light->get_random_point();
      vec3 to_light = on_light - hit.p;

      // get_pdf_value
      float light_area = light->get_area();
      float distance_squared = to_light.length_squared();
      to_light = unit_vector(to_light);
      float light_cosine = fabs(to_light.y);
      float pdf = distance_squared / (light_cosine * light_area);

      ray scattered = ray(hit.p, to_light);

      float scattering_pdf = hit.material_ptr->scatter_pdf(in_ray, hit, scattered);

      vec3 color_from_scatter = (sr.attenuation * scattering_pdf * ray_color(scattered, depth - 1)) / pdf;

      return c_emissive + color_from_scatter;
    }
    else if (option == 1)
    {
      // light ray only
      hittable* light = ajs.scene_root.get_random_light();
      vec3 dir_to_light = unit_vector(light->get_origin() - hit.p);
      cosine_pdf light_pdf = cosine_pdf(dir_to_light);
      ray light_ray = ray(hit.p, light_pdf.get_direction());
      float light_pdf_val = light_pdf.get_value(light_ray.direction);

      float scattering_pdf = hit.material_ptr->scatter_pdf(in_ray, hit, light_ray);

      vec3 c_diffuse = sr.attenuation * scattering_pdf * ray_color(light_ray, depth - 1) / light_pdf_val;
      return c_diffuse;
    }
    else if (option == 2)
    {
      // material ray only
      cosine_pdf material_pdf = cosine_pdf(hit.normal);
      vec3 pdf_direction = material_pdf.get_direction();
      float pdf_val = material_pdf.get_value(pdf_direction);
      ray scattered = ray(hit.p, pdf_direction);

      float scattering_pdf = hit.material_ptr->scatter_pdf(in_ray, hit, scattered);

      vec3 c_diffuse = sr.attenuation * scattering_pdf * ray_color(scattered, depth - 1) / pdf_val;
      return c_diffuse;
    }
    assert(false);
  }
}