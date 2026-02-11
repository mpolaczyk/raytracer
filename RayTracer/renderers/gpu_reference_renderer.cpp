#include "stdafx.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <winerror.h>
#include <algorithm>

#include "math/materials.h"
#include "math/camera.h"
#include "math/hittables.h"
#include "core/string_tools.h"
#include "processing/benchmark.h"
#include "gfx/dx11_helper.h"

#include "gpu_reference_renderer.h"
#include <app/exceptions.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

// GPU-friendly data structures matching HLSL
// Remember vec3 is 16 bytes
struct GPUMaterial
{
  vec3 color;
  vec3 emitted_color;
  vec3 gloss_color;
  uint32_t type;
  float smoothness;
  float gloss_probability;
  float refraction_probability;
  float refraction_index;
  float padding[3];
};

struct GPUSphere
{
  vec3 origin;
  uint32_t material_index;
  float radius;
  float padding[2];
};

struct GPUTriangle
{
  vec3 v0;  // Vertex 0 in world-space coordinates (transformed by static_mesh::pre_render)
  vec3 v1;  // Vertex 1 in world-space coordinates
  vec3 v2;  // Vertex 2 in world-space coordinates
  uint32_t material_index;
  float padding[3];
};

struct GPUCamera
{
  vec3 look_from;
  vec3 lower_left_corner;
  vec3 horizontal;
  vec3 vertical;
  vec3 u;
  vec3 v;
  vec3 w;
  float lens_radius;
  float viewport_width;
  float viewport_height;
  float dist_to_focus;
  float type;
  float padding[3];
};

struct GPUConfig
{
  uint32_t width;
  uint32_t height;
  uint32_t rays_per_pixel;
  uint32_t ray_bounces;
  float white_point;
  float padding[3];
};

namespace
{
constexpr uint32_t kMaxGpuSpheres = 256;
constexpr uint32_t kMaxGpuTriangles = 16384; // Maximum triangles supported (limited by structured buffer size and performance)
constexpr uint32_t kMaxGpuMaterials = 256;
constexpr uint32_t kThreadGroupSize = 8;
constexpr uint32_t kMaxShaderBounces = 16; // Keep in sync with raytracer.hlsl

static_assert(sizeof(GPUMaterial) % 16 == 0, "GPUMaterial must stay 16-byte aligned");
static_assert(sizeof(GPUSphere) % 16 == 0, "GPUSphere must stay 16-byte aligned");
static_assert(sizeof(GPUTriangle) % 16 == 0, "GPUTriangle must stay 16-byte aligned");
static_assert(sizeof(GPUCamera) % 16 == 0, "GPUCamera must stay 16-byte aligned");
static_assert(sizeof(GPUConfig) % 16 == 0, "GPUConfig must stay 16-byte aligned");

bool log_failure(const char* action, HRESULT hr)
{
  if (FAILED(hr))
  {
    logger::error("{} ({})", action, describe_hresult(hr));
    return true;
  }

  return false;
}
}

gpu_reference_renderer::~gpu_reference_renderer()
{
  cleanup_directx();
}

std::string gpu_reference_renderer::get_name() const
{
  return "GPU Reference";
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
      logger::error("{}", "Failed to compile shader");
      return;
    }
  }
  
  // Create output texture
  if (!create_output_texture(job_state.image_width, job_state.image_height))
  {
    logger::error("{}", "Failed to create output texture");
    return;
  }
  
  // Upload scene, camera, and config data to GPU
  if (!upload_scene_data())
  {
    logger::error("{}", "Failed to upload scene data");
    return;
  }
  
  if (!upload_camera_data())
  {
    logger::error("{}", "Failed to upload camera data");
    return;
  }
  
  if (!upload_config_data())
  {
    logger::error("{}", "Failed to upload config data");
    return;
  }
  
  // Set shader and resources
  context->CSSetShader(compute_shader, nullptr, 0);
  context->CSSetConstantBuffers(0, 1, &scene_buffer);
  context->CSSetConstantBuffers(1, 1, &camera_buffer);
  context->CSSetConstantBuffers(2, 1, &config_buffer);
  if (triangle_srv != nullptr)
  {
    context->CSSetShaderResources(0, 1, &triangle_srv);
  }
  context->CSSetUnorderedAccessViews(0, 1, &output_uav, nullptr);
  
  // Dispatch compute shader (8x8 thread groups)
  uint32_t dispatch_x = (job_state.image_width + (kThreadGroupSize - 1)) / kThreadGroupSize;
  uint32_t dispatch_y = (job_state.image_height + (kThreadGroupSize - 1)) / kThreadGroupSize;
  context->Dispatch(dispatch_x, dispatch_y, 1);
  
  // Unbind UAV and SRV
  ID3D11UnorderedAccessView* null_uav = nullptr;
  context->CSSetUnorderedAccessViews(0, 1, &null_uav, nullptr);
  if (triangle_srv != nullptr)
  {
    ID3D11ShaderResourceView* null_srv = nullptr;
    context->CSSetShaderResources(0, 1, &null_srv);
  }
  
  // Read back results and apply tone mapping
  if (!readback_results())
  {
    logger::error("{}", "Failed to readback results");
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
    logger::error("{}", "Failed to use device");
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
  if (triangle_srv) { triangle_srv->Release(); triangle_srv = nullptr; }
  if (triangle_buffer) { triangle_buffer->Release(); triangle_buffer = nullptr; }
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

  flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;


  const std::wstring shader = L"raytracer.hlsl";
  HRESULT hr = D3DCompileFromFile(
    shader.c_str(),
    nullptr,
    D3D_COMPILE_STANDARD_FILE_INCLUDE,
    "CSMain",
    "cs_5_0",
    flags,
    0,
    &shader_blob,
    &error_blob
  );

  std::ostringstream oss;
  if (FAILED(hr))
  {
    oss << describe_hresult(hr);
    if (error_blob)
    {
      oss << static_cast<const char*>(error_blob->GetBufferPointer());
      error_blob->Release();
      error_blob = nullptr;
    }
    logger::error("{}", oss.str());
    if (shader_blob)
    {
      shader_blob->Release();
    }
    return false;
  }
  
  if (error_blob)
  {
    const char* warnings = static_cast<const char*>(error_blob->GetBufferPointer());
    if (warnings && warnings[0] != '\0')
    {
      logger::warn("Shader compile warnings:\n{}", warnings);
    }
    error_blob->Release();
    error_blob = nullptr;
  }

  hr = device->CreateComputeShader(
    shader_blob->GetBufferPointer(),
    shader_blob->GetBufferSize(),
    nullptr,
    &compute_shader
  );

  if (log_failure("CreateComputeShader", hr))
  {
    shader_blob->Release();
    return false;
  }

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
  std::vector<GPUTriangle> gpu_triangles;
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
          GPUMaterial gpu_mat{};
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
      GPUSphere gpu_sphere{};
      gpu_sphere.origin = sph->origin;
      gpu_sphere.radius = sph->radius;
      gpu_sphere.material_index = mat_index;
      gpu_spheres.push_back(gpu_sphere);
    }
    else if (obj->type == hittable_type::static_mesh)
    {
      static_mesh* mesh = static_cast<static_mesh*>(obj);
      
      // Get or create material index
      uint32_t mat_index = 0;
      if (mesh->material_ptr != nullptr)
      {
        auto it = material_index_map.find(mesh->material_ptr);
        if (it == material_index_map.end())
        {
          mat_index = static_cast<uint32_t>(gpu_materials.size());
          material_index_map[mesh->material_ptr] = mat_index;

          // Convert material to GPU format
          GPUMaterial gpu_mat{};
          gpu_mat.color = mesh->material_ptr->color;
          gpu_mat.emitted_color = mesh->material_ptr->emitted_color;
          gpu_mat.smoothness = mesh->material_ptr->smoothness;
          gpu_mat.gloss_probability = mesh->material_ptr->gloss_probability;
          gpu_mat.gloss_color = mesh->material_ptr->gloss_color;
          gpu_mat.refraction_probability = mesh->material_ptr->refraction_probability;
          gpu_mat.refraction_index = mesh->material_ptr->refraction_index;
          gpu_mat.type = static_cast<uint32_t>(mesh->material_ptr->type);
          gpu_materials.push_back(gpu_mat);
        }
        else
        {
          mat_index = it->second;
        }
      }

      // Convert triangles to GPU format
      for (const auto& face : mesh->faces)
      {
        GPUTriangle gpu_tri{};
        gpu_tri.v0 = face.vertices[0];
        gpu_tri.v1 = face.vertices[1];
        gpu_tri.v2 = face.vertices[2];
        gpu_tri.material_index = mat_index;
        gpu_triangles.push_back(gpu_tri);
      }
    }
    else if (obj->type == hittable_type::xy_rect || obj->type == hittable_type::xz_rect || obj->type == hittable_type::yz_rect)
    {
      // Get or create material index
      uint32_t mat_index = 0;
      if (obj->material_ptr != nullptr)
      {
        auto it = material_index_map.find(obj->material_ptr);
        if (it == material_index_map.end())
        {
          mat_index = static_cast<uint32_t>(gpu_materials.size());
          material_index_map[obj->material_ptr] = mat_index;

          // Convert material to GPU format
          GPUMaterial gpu_mat{};
          gpu_mat.color = obj->material_ptr->color;
          gpu_mat.emitted_color = obj->material_ptr->emitted_color;
          gpu_mat.smoothness = obj->material_ptr->smoothness;
          gpu_mat.gloss_probability = obj->material_ptr->gloss_probability;
          gpu_mat.gloss_color = obj->material_ptr->gloss_color;
          gpu_mat.refraction_probability = obj->material_ptr->refraction_probability;
          gpu_mat.refraction_index = obj->material_ptr->refraction_index;
          gpu_mat.type = static_cast<uint32_t>(obj->material_ptr->type);
          gpu_materials.push_back(gpu_mat);
        }
        else
        {
          mat_index = it->second;
        }
      }

      auto add_triangle = [&](const vec3& v0, const vec3& v1, const vec3& v2)
      {
        GPUTriangle gpu_tri{};
        gpu_tri.v0 = v0;
        gpu_tri.v1 = v1;
        gpu_tri.v2 = v2;
        gpu_tri.material_index = mat_index;
        gpu_triangles.push_back(gpu_tri);
      };

      if (obj->type == hittable_type::xy_rect)
      {
        xy_rect* rect = static_cast<xy_rect*>(obj);
        const vec3 v0(rect->x0, rect->y0, rect->z);
        const vec3 v1(rect->x1, rect->y0, rect->z);
        const vec3 v2(rect->x1, rect->y1, rect->z);
        const vec3 v3(rect->x0, rect->y1, rect->z);
        add_triangle(v0, v1, v2);
        add_triangle(v0, v2, v3);
      }
      else if (obj->type == hittable_type::xz_rect)
      {
        xz_rect* rect = static_cast<xz_rect*>(obj);
        const vec3 v0(rect->x0, rect->y, rect->z0);
        const vec3 v1(rect->x0, rect->y, rect->z1);
        const vec3 v2(rect->x1, rect->y, rect->z1);
        const vec3 v3(rect->x1, rect->y, rect->z0);
        add_triangle(v0, v1, v2);
        add_triangle(v0, v2, v3);
      }
      else if (obj->type == hittable_type::yz_rect)
      {
        yz_rect* rect = static_cast<yz_rect*>(obj);
        const vec3 v0(rect->x, rect->y0, rect->z0);
        const vec3 v1(rect->x, rect->y1, rect->z0);
        const vec3 v2(rect->x, rect->y1, rect->z1);
        const vec3 v3(rect->x, rect->y0, rect->z1);
        add_triangle(v0, v1, v2);
        add_triangle(v0, v2, v3);
      }
    }
  }
  
  if (gpu_spheres.size() > kMaxGpuSpheres)
  {
    logger::error("Scene uses {} spheres but the GPU shader only supports {}", gpu_spheres.size(), kMaxGpuSpheres);
    return false;
  }
  
  if (gpu_triangles.size() > kMaxGpuTriangles)
  {
    logger::error("Scene uses {} triangles but the GPU shader only supports {}", gpu_triangles.size(), kMaxGpuTriangles);
    return false;
  }
  
  if (gpu_materials.size() > kMaxGpuMaterials)
  {
    logger::error("Scene uses {} materials but the GPU shader only supports {}", gpu_materials.size(), kMaxGpuMaterials);
    return false;
  }

  // Create scene buffer (spheres + materials + counts)
  struct SceneData
  {
    GPUSphere spheres[kMaxGpuSpheres];
    GPUMaterial materials[kMaxGpuMaterials];
    uint32_t sphere_count;
    uint32_t material_count;
    uint32_t triangle_count;
    float padding[1];
  };

  static_assert(sizeof(SceneData) % 16 == 0, "Scene constant buffer must be 16-byte aligned");
  static_assert(sizeof(SceneData) <= D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16, "Scene constant buffer exceeds D3D11 limits");
 
  SceneData scene_data{};

  scene_data.sphere_count = static_cast<uint32_t>(gpu_spheres.size());
  scene_data.triangle_count = static_cast<uint32_t>(gpu_triangles.size());
  scene_data.material_count = static_cast<uint32_t>(gpu_materials.size());

  for (size_t i = 0; i < gpu_spheres.size() && i < kMaxGpuSpheres; ++i)
  {
    scene_data.spheres[i] = gpu_spheres[i];
  }

  for (size_t i = 0; i < gpu_materials.size() && i < kMaxGpuMaterials; ++i)
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
    if (log_failure("Create scene constant buffer", hr))
    {
      return false;
    }
  }
  else
  {
    context->UpdateSubresource(scene_buffer, 0, nullptr, &scene_data, 0, 0);
  }

  // Create triangle structured buffer
  if (scene_data.triangle_count > 0)
  {
    // Clean up old triangle buffer if it exists
    if (triangle_srv) { triangle_srv->Release(); triangle_srv = nullptr; }
    if (triangle_buffer) { triangle_buffer->Release(); triangle_buffer = nullptr; }

    // Create structured buffer for triangles
    D3D11_BUFFER_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = sizeof(GPUTriangle) * scene_data.triangle_count;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    desc.StructureByteStride = sizeof(GPUTriangle);

    D3D11_SUBRESOURCE_DATA init_data;
    init_data.pSysMem = gpu_triangles.data();
    init_data.SysMemPitch = 0;
    init_data.SysMemSlicePitch = 0;

    HRESULT hr = device->CreateBuffer(&desc, &init_data, &triangle_buffer);
    if (log_failure("Create triangle structured buffer", hr))
    {
      return false;
    }

    // Create shader resource view for the structured buffer
    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
    ZeroMemory(&srv_desc, sizeof(srv_desc));
    srv_desc.Format = DXGI_FORMAT_UNKNOWN;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srv_desc.Buffer.FirstElement = 0;
    srv_desc.Buffer.NumElements = scene_data.triangle_count;

    hr = device->CreateShaderResourceView(triangle_buffer, &srv_desc, &triangle_srv);
    if (log_failure("Create triangle SRV", hr))
    {
      return false;
    }
  }

  logger::debug("Uploaded {} spheres / {} triangles / {} materials to GPU", scene_data.sphere_count, scene_data.triangle_count, scene_data.material_count);

  return true;
}

bool gpu_reference_renderer::upload_camera_data()
{
  assert(job_state.cam != nullptr);

  // Convert camera to GPU format using the getters
  camera* cam = job_state.cam;
  
  GPUCamera gpu_camera{};

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
    if (log_failure("Create camera constant buffer", hr))
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
  if (job_state.renderer_conf == nullptr)
  {
    logger::error("Renderer configuration missing for GPU upload");
    return false;
  }
  
  GPUConfig gpu_config;
  gpu_config.width = job_state.image_width;
  gpu_config.height = job_state.image_height;
  gpu_config.rays_per_pixel = job_state.renderer_conf->rays_per_pixel;
  gpu_config.ray_bounces = job_state.renderer_conf->ray_bounces;
  gpu_config.white_point = job_state.renderer_conf->white_point;
  
  if (gpu_config.ray_bounces > kMaxShaderBounces)
  {
    logger::warn("Clamping ray bounce count from {} to shader maximum {}", gpu_config.ray_bounces, kMaxShaderBounces);
    gpu_config.ray_bounces = kMaxShaderBounces;
  }
  
  if (gpu_config.rays_per_pixel == 0)
  {
    logger::warn("Rays per pixel was zero. Forcing minimum of 1 to avoid GPU division by zero");
    gpu_config.rays_per_pixel = 1;
  }

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
    if (log_failure("Create config constant buffer", hr))
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
    if (log_failure("Create output texture", hr))
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
    if (log_failure("Create output UAV", hr))
    {
      return false;
    }

    // Create staging texture for readback
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

    hr = device->CreateTexture2D(&desc, nullptr, &staging_texture);
    if (log_failure("Create staging texture", hr))
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
  if (log_failure("Map staging texture", hr))
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
