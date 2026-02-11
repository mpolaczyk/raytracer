#pragma once

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;
struct ID3D11Query;

namespace dx11
{
  struct gpu_timer
  {
    ID3D11Query* disjoint_query = nullptr;
    ID3D11Query* start_query = nullptr;
    ID3D11Query* end_query = nullptr;
  };

  extern ID3D11Device* g_pd3dDevice;
  extern ID3D11DeviceContext* g_pd3dDeviceContext;
  extern IDXGISwapChain* g_pSwapChain;
  extern ID3D11RenderTargetView* g_mainRenderTargetView;
  extern bool g_debugLayerEnabled;

  bool CreateDeviceD3D(HWND hWnd);
  void CleanupDeviceD3D();
  void CreateRenderTarget();
  void CleanupRenderTarget();
  
  bool UpdateTextureBuffer(unsigned char* buffer, int width, int height, ID3D11Texture2D* in_texture);
  bool LoadTextureFromBuffer(unsigned char* buffer, int width, int height, ID3D11ShaderResourceView** out_srv, ID3D11Texture2D** out_texture);
  bool LoadTextureFromFile(const char* filename, int& out_width, int& out_height, ID3D11ShaderResourceView** out_srv, ID3D11Texture2D** out_texture);
  
  bool InitializeGpuTimer(gpu_timer& timer);
  void BeginGpuTimer(gpu_timer& timer);
  void EndGpuTimer(gpu_timer& timer);
  bool ReadGpuTimeMs(gpu_timer& timer, double& out_ms);
  void ReleaseGpuTimer(gpu_timer& timer);
}