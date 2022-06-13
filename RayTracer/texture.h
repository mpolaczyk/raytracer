#pragma once

#include "vec3.h"

class texture 
{
public:
  virtual vec3 value(float u, float v, const vec3& p) const = 0;
};

class solid_texture : public texture
{
public:
  solid_texture() {}
  solid_texture(const vec3 color) : color(color) {}
  solid_texture(float r, float g, float b) : solid_texture(vec3(r, g, b)) {}

  virtual vec3 value(float u, float v, const vec3& p) const override
  {
    return color;
  }

private:
  vec3 color;
};

class checker_texture : public texture 
{
public:
  checker_texture() {}
  checker_texture(texture* even, texture* odd) : even(even), odd(odd) {}
 
  virtual vec3 value(float u, float v, const vec3& p) const override
  {
    float sines = sin(10 * p.x) * sin(10 * p.y) * sin(10 * p.z);
    if (sines < 0) return odd->value(u, v, p);
    else return even->value(u, v, p);
  }

public:
  texture* odd = nullptr;
  texture* even = nullptr;
};