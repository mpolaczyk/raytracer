#pragma once

#include "processing/async_renderer_base.h"

class preview_normals_renderer : public async_renderer_base
{
public:
  virtual std::string get_name() const override;
  virtual renderer_type get_renderer_type() const override { return renderer_type::preview_normals; }

private:
  virtual void render() override;

  void render_chunk(const chunk& in_chunk);
};