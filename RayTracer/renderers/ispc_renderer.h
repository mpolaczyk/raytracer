#pragma once

#include "processing/async_renderer_base.h"

#include "renderers/trace_ray.h"

class ispc_renderer : public async_renderer_base
{
public:
  virtual std::string get_name() const override;
  virtual renderer_type get_renderer_type() const override { return renderer_type::ispc; }

private:
  virtual void render() override;

  void render_chunk(const chunk& in_chunk, ispc::float3* output);
};