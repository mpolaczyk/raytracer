#pragma once

#include "processing/async_renderer_base.h"
#include "gfx/dx11_helper.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11ComputeShader;
struct ID3D11Buffer;
struct ID3D11Query;
struct ID3D11UnorderedAccessView;
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;

class gpu_reference_renderer : public async_renderer_base
{
public:
  gpu_reference_renderer() = default;
  virtual ~gpu_reference_renderer();

  virtual std::string get_name() const override;
  virtual renderer_type get_renderer_type() const override { return renderer_type::gpu_reference; }
  ID3D11ShaderResourceView* get_output_srv() const { return output_srv; }
  ID3D11Texture2D* get_output_texture() const { return output_texture; }

private:
  virtual void render() override;
  virtual bool wants_sync_render() const override { return false; }

  // DirectX 11 resources
  ID3D11Device* device = nullptr;
  ID3D11DeviceContext* context = nullptr;
  ID3D11ComputeShader* compute_shader = nullptr;
  ID3D11Buffer* scene_buffer = nullptr;
  ID3D11Buffer* triangle_buffer = nullptr;
  ID3D11ShaderResourceView* triangle_srv = nullptr;
  ID3D11Buffer* camera_buffer = nullptr;
  ID3D11Buffer* config_buffer = nullptr;
  ID3D11UnorderedAccessView* output_uav = nullptr;
  ID3D11ShaderResourceView* output_srv = nullptr;
  ID3D11Texture2D* output_texture = nullptr;
  ID3D11Texture2D* staging_texture = nullptr;
  dx11::gpu_timer gpu_timer;

  // Helper methods
  bool initialize_directx();
  void cleanup_directx();
  bool compile_shader();
  bool upload_scene_data();
  bool upload_camera_data();
  bool upload_config_data();
  bool create_output_texture(int width, int height);
  bool readback_results();
  void log_gpu_time();
};
