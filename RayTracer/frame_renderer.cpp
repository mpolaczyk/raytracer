#include "frame_renderer.h"

#include <functional>
#include <assert.h>

#include "camera.h"
#include "common.h"

using namespace bmp;
using namespace std;

frame_renderer::frame_renderer(int width, int height, camera* cam)
  : image_width(width), image_height(height), cam(cam)
{
  assert(cam != nullptr);

  img = new bmp_image(image_width, image_height);

  cout << "Frame renderer: " << image_width << "x" << image_height << endl;

  samples_per_pixel = 10;
  for (int s = 0; s < samples_per_pixel; s++)
  {
    random_floats.push_back(random_float());
  }
}
frame_renderer::~frame_renderer()
{
  delete img;
}

color3 inline frame_renderer::ray_color(const ray& r, const hittable_list& world, int depth)
{
  if (depth <= 0)
  {
    return color3(0, 0, 0);
  }

  hit_record rec;
  if (world.hit(r, 0.001f, infinity, rec))
  {
    point3 target = rec.p + rec.normal + random_in_unit_sphere();
    return 0.5 * ray_color(ray(rec.p, target - rec.p), world, depth - 1);
  }
  vec3 unit_direction = unit_vector(r.direction());
  float t = 0.5f * (unit_direction.y() + 1.0f);
  return (1.0f - t) * white + t * blue;
}

void frame_renderer::render(const hittable_list& world_list)
{
  assert(cam != nullptr);
  assert(img != nullptr);

  const int max_depth = 50;

  for (int y = 0; y < image_height; ++y)
  {
    for (int x = 0; x < image_width; ++x)
    {
      color3 pixel_color;
      for (float s : random_floats)
      {
        float u = float(x + s) / (image_width - 1);
        float v = float(y + s) / (image_height - 1);
        ray r = cam->get_ray(u, v);
        pixel_color += ray_color(r, world_list, max_depth);
      }
      bmp_pixel p = bmp_pixel(pixel_color / (float)samples_per_pixel);
      img->draw_pixel(x, y, &p);
    }
  }
}

void frame_renderer::save(const char* file_name)
{
  img->save_to_file(file_name);
}