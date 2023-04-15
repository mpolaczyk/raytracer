#pragma once

#include "processing/async_renderer_base.h"

class reference_renderer : public async_renderer_base
{
public:
  virtual std::string get_name() const override;

private:
  virtual void render() override;

  void render_chunk(const tchunk& in_chunk);

  vec3 fragment(float x, float y, const vec3& resolution);

  vec3 enviroment_light(const ray& in_ray);

  vec3 ray_color(ray in_ray, uint32_t seed);
};