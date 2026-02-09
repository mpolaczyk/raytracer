#include "stdafx.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <winerror.h>

#include "math/materials.h"
#include "math/camera.h"
#include "math/hittables.h"
#include "processing/benchmark.h"
#include "gfx/dx11_helper.h"

#include "gpu_reference_renderer.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

// GPU-friendly data structures matching HLSL
struct GPUMaterial
{
  vec3 color;
  float smoothness;
  vec3 emitted_color;
  float gloss_probability;
  vec3 gloss_color;
  float refraction_probability;
  float refraction_index;
  uint32_t type;
  vec3 padding; // Alignment padding
};

struct GPUSphere
{
  vec3 origin;
  float radius;
  uint32_t material_index;
  vec3 padding; // Alignment padding
};

struct GPUCamera
{
  vec3 look_from;
  float lens_radius;
  vec3 lower_left_corner;
  float viewport_width;
  vec3 horizontal;
  float viewport_height;
  vec3 vertical;
  float dist_to_focus;
  vec3 u;
  float type;
  vec3 v;
  float padding;
  vec3 w;
  float padding2;
};

struct GPUConfig
{
  uint32_t width;
  uint32_t height;
  uint32_t rays_per_pixel;
  uint32_t ray_bounces;
  float white_point;
  vec3 padding; // Alignment padding
};

gpu_reference_renderer::gpu_reference_renderer()
{
}

gpu_reference_renderer::~gpu_reference_renderer()
{
  cleanup_directx();
}

std::string gpu_reference_renderer::get_name() const
{
  return "GPU Reference (DirectX 11)";
}

void gpu_reference_renderer::render()
{
  save_output = true;

  benchmark::scope_counter benchmark_render("GPU render", false);

  // Initialize DirectX if needed
  if (device == nullptr)
  {
    if (!initialize_directx())
    {
      return;
    }
  }

  // Compile shader if needed
  if (compute_shader == nullptr)
  {
    if (!compile_shader())
    {
      return;
    }
  }

  // Create output texture
  if (!create_output_texture(job_state.image_width, job_state.image_height))
  {
    return;
  }

  // Upload scene, camera, and config data to GPU
  if (!upload_scene_data())
  {
    return;
  }

  if (!upload_camera_data())
  {
    return;
  }

  if (!upload_config_data())
  {
    return;
  }

  // Set shader and resources
  context->CSSetShader(compute_shader, nullptr, 0);
  context->CSSetConstantBuffers(0, 1, &scene_buffer);
  context->CSSetConstantBuffers(1, 1, &camera_buffer);
  context->CSSetConstantBuffers(2, 1, &config_buffer);
  context->CSSetUnorderedAccessViews(0, 1, &output_uav, nullptr);

  // Dispatch compute shader (8x8 thread groups)
  uint32_t dispatch_x = (job_state.image_width + 7) / 8;
  uint32_t dispatch_y = (job_state.image_height + 7) / 8;
  context->Dispatch(dispatch_x, dispatch_y, 1);

  // Unbind UAV
  ID3D11UnorderedAccessView* null_uav = nullptr;
  context->CSSetUnorderedAccessViews(0, 1, &null_uav, nullptr);

  // Read back results and apply tone mapping
  if (!readback_results())
  {
    return;
  }
}

bool gpu_reference_renderer::initialize_directx()
{
  // Use the global DirectX device from dx11_helper
  device = dx11::g_pd3dDevice;
  context = dx11::g_pd3dDeviceContext;

  if (device == nullptr || context == nullptr)
  {
    return false;
  }

  return true;
}

void gpu_reference_renderer::cleanup_directx()
{
  if (staging_texture) { staging_texture->Release(); staging_texture = nullptr; }
  if (output_texture) { output_texture->Release(); output_texture = nullptr; }
  if (output_uav) { output_uav->Release(); output_uav = nullptr; }
  if (config_buffer) { config_buffer->Release(); config_buffer = nullptr; }
  if (camera_buffer) { camera_buffer->Release(); camera_buffer = nullptr; }
  if (scene_buffer) { scene_buffer->Release(); scene_buffer = nullptr; }
  if (compute_shader) { compute_shader->Release(); compute_shader = nullptr; }
  
  // Don't release device and context - they're owned by dx11_helper
  device = nullptr;
  context = nullptr;
}

bool gpu_reference_renderer::compile_shader()
{
  ID3DBlob* shader_blob = nullptr;
  ID3DBlob* error_blob = nullptr;

  UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
  flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

  HRESULT hr = D3DCompileFromFile(
    L"renderers/raytracer.hlsl",
    nullptr,
    D3D_COMPILE_STANDARD_FILE_INCLUDE,
    "CSMain",
    "cs_5_0",
    flags,
    0,
    &shader_blob,
    &error_blob
  );

  if (FAILED(hr))
  {
    if (error_blob)
    {
      error_blob->Release();
    }
    return false;
  }

  hr = device->CreateComputeShader(
    shader_blob->GetBufferPointer(),
    shader_blob->GetBufferSize(),
    nullptr,
    &compute_shader
  );

  shader_blob->Release();

  return SUCCEEDED(hr);
}

bool gpu_reference_renderer::upload_scene_data()
{
  assert(job_state.scene_root != nullptr);

  // Get the scene
  scene* sc = job_state.scene_root;

  // Count spheres in the scene
  std::vector<GPUSphere> gpu_spheres;
  std::vector<GPUMaterial> gpu_materials;
  std::map<material*, uint32_t> material_index_map;

  for (auto* obj : sc->objects)
  {
    if (obj->type == hittable_type::sphere)
    {
      sphere* sph = static_cast<sphere*>(obj);
      
      // Get or create material index
      uint32_t mat_index = 0;
      if (sph->material_ptr != nullptr)
      {
        auto it = material_index_map.find(sph->material_ptr);
        if (it == material_index_map.end())
        {
          mat_index = static_cast<uint32_t>(gpu_materials.size());
          material_index_map[sph->material_ptr] = mat_index;

          // Convert material to GPU format
          GPUMaterial gpu_mat;
          gpu_mat.color = sph->material_ptr->color;
          gpu_mat.emitted_color = sph->material_ptr->emitted_color;
          gpu_mat.smoothness = sph->material_ptr->smoothness;
          gpu_mat.gloss_probability = sph->material_ptr->gloss_probability;
          gpu_mat.gloss_color = sph->material_ptr->gloss_color;
          gpu_mat.refraction_probability = sph->material_ptr->refraction_probability;
          gpu_mat.refraction_index = sph->material_ptr->refraction_index;
          gpu_mat.type = static_cast<uint32_t>(sph->material_ptr->type);
          gpu_materials.push_back(gpu_mat);
        }
        else
        {
          mat_index = it->second;
        }
      }

      // Convert sphere to GPU format
      GPUSphere gpu_sphere;
      gpu_sphere.origin = sph->origin;
      gpu_sphere.radius = sph->radius;
      gpu_sphere.material_index = mat_index;
      gpu_spheres.push_back(gpu_sphere);
    }
  }

  // Create scene buffer (spheres + materials + counts)
  struct SceneData
  {
    GPUSphere spheres[256];
    GPUMaterial materials[256];
    uint32_t sphere_count;
    uint32_t material_count;
    float padding[2];
  };

  SceneData scene_data;
  memset(&scene_data, 0, sizeof(SceneData));

  scene_data.sphere_count = static_cast<uint32_t>(gpu_spheres.size());
  scene_data.material_count = static_cast<uint32_t>(gpu_materials.size());

  for (size_t i = 0; i < gpu_spheres.size() && i < 256; ++i)
  {
    scene_data.spheres[i] = gpu_spheres[i];
  }

  for (size_t i = 0; i < gpu_materials.size() && i < 256; ++i)
  {
    scene_data.materials[i] = gpu_materials[i];
  }

  // Create or update buffer
  if (scene_buffer == nullptr)
  {
    D3D11_BUFFER_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = sizeof(SceneData);
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    D3D11_SUBRESOURCE_DATA init_data;
    init_data.pSysMem = &scene_data;
    init_data.SysMemPitch = 0;
    init_data.SysMemSlicePitch = 0;

    HRESULT hr = device->CreateBuffer(&desc, &init_data, &scene_buffer);
    if (FAILED(hr))
    {
      return false;
    }
  }
  else
  {
    context->UpdateSubresource(scene_buffer, 0, nullptr, &scene_data, 0, 0);
  }

  return true;
}

bool gpu_reference_renderer::upload_camera_data()
{
  assert(job_state.cam != nullptr);

  // Convert camera to GPU format using the getters
  camera* cam = job_state.cam;
  
  GPUCamera gpu_camera;
  ZeroMemory(&gpu_camera, sizeof(GPUCamera));

  // Extract camera data
  gpu_camera.look_from = cam->get_look_from();
  gpu_camera.lens_radius = cam->get_lens_radius();
  gpu_camera.lower_left_corner = cam->get_lower_left_corner();
  gpu_camera.horizontal = cam->get_horizontal();
  gpu_camera.vertical = cam->get_vertical();
  gpu_camera.viewport_width = cam->get_viewport_width();
  gpu_camera.viewport_height = cam->get_viewport_height();
  gpu_camera.dist_to_focus = cam->get_dist_to_focus();
  gpu_camera.u = cam->get_u();
  gpu_camera.v = cam->get_v();
  gpu_camera.w = cam->get_w();
  gpu_camera.type = cam->get_type();

  // Create or update buffer
  if (camera_buffer == nullptr)
  {
    D3D11_BUFFER_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = sizeof(GPUCamera);
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    D3D11_SUBRESOURCE_DATA init_data;
    init_data.pSysMem = &gpu_camera;
    init_data.SysMemPitch = 0;
    init_data.SysMemSlicePitch = 0;

    HRESULT hr = device->CreateBuffer(&desc, &init_data, &camera_buffer);
    if (FAILED(hr))
    {
      return false;
    }
  }
  else
  {
    context->UpdateSubresource(camera_buffer, 0, nullptr, &gpu_camera, 0, 0);
  }

  return true;
}

bool gpu_reference_renderer::upload_config_data()
{
  GPUConfig gpu_config;
  gpu_config.width = job_state.image_width;
  gpu_config.height = job_state.image_height;
  gpu_config.rays_per_pixel = job_state.renderer_conf->rays_per_pixel;
  gpu_config.ray_bounces = job_state.renderer_conf->ray_bounces;
  gpu_config.white_point = job_state.renderer_conf->white_point;

  // Create or update buffer
  if (config_buffer == nullptr)
  {
    D3D11_BUFFER_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = sizeof(GPUConfig);
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    D3D11_SUBRESOURCE_DATA init_data;
    init_data.pSysMem = &gpu_config;
    init_data.SysMemPitch = 0;
    init_data.SysMemSlicePitch = 0;

    HRESULT hr = device->CreateBuffer(&desc, &init_data, &config_buffer);
    if (FAILED(hr))
    {
      return false;
    }
  }
  else
  {
    context->UpdateSubresource(config_buffer, 0, nullptr, &gpu_config, 0, 0);
  }

  return true;
}

bool gpu_reference_renderer::create_output_texture(int width, int height)
{
  // Clean up old textures if they exist and size changed
  if (output_texture != nullptr)
  {
    // Check if size has changed
    D3D11_TEXTURE2D_DESC desc;
    output_texture->GetDesc(&desc);
    
    if (desc.Width != static_cast<UINT>(width) || desc.Height != static_cast<UINT>(height))
    {
      if (output_uav) { output_uav->Release(); output_uav = nullptr; }
      if (output_texture) { output_texture->Release(); output_texture = nullptr; }
      if (staging_texture) { staging_texture->Release(); staging_texture = nullptr; }
    }
  }

  if (output_texture == nullptr)
  {
    // Create output texture with UAV
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;

    HRESULT hr = device->CreateTexture2D(&desc, nullptr, &output_texture);
    if (FAILED(hr))
    {
      return false;
    }

    // Create UAV
    D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
    ZeroMemory(&uav_desc, sizeof(uav_desc));
    uav_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    uav_desc.Texture2D.MipSlice = 0;

    hr = device->CreateUnorderedAccessView(output_texture, &uav_desc, &output_uav);
    if (FAILED(hr))
    {
      return false;
    }

    // Create staging texture for readback
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

    hr = device->CreateTexture2D(&desc, nullptr, &staging_texture);
    if (FAILED(hr))
    {
      return false;
    }
  }

  return true;
}

bool gpu_reference_renderer::readback_results()
{
  // Copy from output texture to staging texture
  context->CopyResource(staging_texture, output_texture);

  // Map staging texture for CPU read
  D3D11_MAPPED_SUBRESOURCE mapped;
  HRESULT hr = context->Map(staging_texture, 0, D3D11_MAP_READ, 0, &mapped);
  if (FAILED(hr))
  {
    return false;
  }

  // Copy data to output buffers
  float* src = reinterpret_cast<float*>(mapped.pData);

  for (int y = 0; y < job_state.image_height; ++y)
  {
    for (int x = 0; x < job_state.image_width; ++x)
    {
      int src_offset = (y * mapped.RowPitch / sizeof(float)) + (x * 4);
      
      float r = src[src_offset + 0];
      float g = src[src_offset + 1];
      float b = src[src_offset + 2];

      // The tone mapping is already done in the shader
      // Convert to 8-bit color
      vec3 ldr_color(r, g, b);
      ldr_color = math::clamp_vec3(0.0f, 1.0f, ldr_color);

      bmp::bmp_pixel p(ldr_color);
      job_state.img_rgb->draw_pixel(x, y, &p, bmp::bmp_format::rgba);
      if (save_output)
      {
        job_state.img_bgr->draw_pixel(x, y, &p);
      }
    }
  }

  context->Unmap(staging_texture, 0);

  return true;
}
