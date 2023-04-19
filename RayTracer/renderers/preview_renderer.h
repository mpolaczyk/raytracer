#pragma once

#include "processing/async_renderer_base.h"

class preview_renderer : public async_renderer_base
{
public:
  virtual std::string get_name() const override;

private:
  virtual void render() override;

  void render_chunk(const tchunk& in_chunk);
};