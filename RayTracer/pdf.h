#pragma once

#include "common.h"
#include "onb.h"

struct pdf 
{
  virtual vec3 generate() const = 0;

  virtual float value(const vec3& direction) const = 0;
};


struct sphere_pdf : public pdf
{
  sphere_pdf() {}

  vec3 generate() const override
  {
    return random_cache::get_vec3();
  }

  float value(const vec3& direction) const override
  {
    return 1.0f / (4.0f * pi);
  }
};

struct cosine_pdf : public pdf
{
  cosine_pdf() {}

  cosine_pdf(const vec3& w) 
  { 
    uvw.build_from_w(w); 
  }

  vec3 generate() const override
  {
    return uvw.local(random_cosine_direction());
  }

  float value(const vec3& direction) const override
  {
    float cosine_theta = dot(unit_vector(direction), uvw.w);
    return fmax(0.0f, cosine_theta / pi);
  }

  onb uvw;
};
