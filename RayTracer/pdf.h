#pragma once

#include "common.h"
#include "onb.h"

struct pdf 
{
  // Return a random direction weighted by the internal PDF distribution
  virtual vec3 get_direction() const = 0;

  // Return the corresponding PDF distribution value in that direction
  virtual float get_value(const vec3& direction) const = 0;
};


struct sphere_pdf : public pdf
{
  sphere_pdf() {}

  virtual vec3 get_direction() const override
  {
    return random_cache::get_vec3();
  }

  virtual float get_value(const vec3& direction) const override
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

  virtual vec3 get_direction() const override
  {
    return uvw.local(random_cosine_direction());
  }

  virtual float get_value(const vec3& direction) const override
  {
    float cosine_theta = dot(unit_vector(direction), uvw.w);
    return (cosine_theta <= 0) ? 0 : cosine_theta / pi;
  }

  onb uvw;
};

struct hittable_pdf : public pdf
{
public:
  hittable_pdf(const hittable* _objects, const vec3& _origin)
    : object(_objects), origin(_origin)
  {}

  virtual vec3 get_direction() const override
  {
    return object->get_pdf_direction(origin);
  }

  virtual float get_value(const vec3& direction) const override
  {
    return object->get_pdf_value(origin, direction);
  }

public:
  const hittable* object;
  vec3 origin;
};

class mixture_pdf : public pdf
{
public:
  mixture_pdf(pdf* p0, pdf* p1, float ratio)
    : ratio(ratio)
  {
    p[0] = p0;
    p[1] = p1;
  }

  float get_value(const vec3& direction) const override
  {
    return 0.5f * p[0]->get_value(direction)
         + 0.5f * p[1]->get_value(direction);
  }

  vec3 get_direction() const override
  {
    if (random_cache::get_float_0_1() > ratio)
      return p[0]->get_direction();
    else
      return p[1]->get_direction();
  }

public:
  pdf* p[2];
  float ratio = 0.5f;
};