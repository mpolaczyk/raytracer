#pragma once

#include <d3d11.h>

namespace dx11
{
  extern ID3D11Device* g_pd3dDevice;
  extern ID3D11DeviceContext* g_pd3dDeviceContext;
  extern IDXGISwapChain* g_pSwapChain;
  extern ID3D11RenderTargetView* g_mainRenderTargetView;

  bool CreateDeviceD3D(HWND hWnd);
  void CleanupDeviceD3D();
  void CreateRenderTarget();
  void CleanupRenderTarget();
  LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
  bool UpdateTextureBuffer(unsigned char* buffer, int width, int height, ID3D11Texture2D* in_texture);
  bool LoadTextureFromBuffer(unsigned char* buffer, int width, int height, ID3D11ShaderResourceView** out_srv, ID3D11Texture2D** out_texture);
  bool LoadTextureFromFile(const char* filename, int& out_width, int& out_height, ID3D11ShaderResourceView** out_srv, ID3D11Texture2D** out_texture);
}