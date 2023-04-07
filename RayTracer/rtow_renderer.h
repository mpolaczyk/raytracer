#pragma once

#include "async_renderer_base.h"

// Based on Ray Tracing in One Weekend https://raytracing.github.io/ by Peter Shirley
class rtow_renderer : public async_renderer_base
{
public:
  virtual std::string get_name() const override;

private:
  virtual void render() override;

  void render_chunk(const chunk& in_chunk);
  vec3 ray_color(const ray& in_ray, uint32_t depth);
};