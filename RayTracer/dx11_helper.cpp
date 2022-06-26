#include "stdafx.h"

#include <d3d11.h>
#include <winerror.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace dx11
{
  ID3D11Device* g_pd3dDevice = nullptr;
  ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
  IDXGISwapChain* g_pSwapChain = nullptr;
  ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

  void CreateRenderTarget()
  {
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
  }

  bool CreateDeviceD3D(HWND hWnd)
  {
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
      return false;

    CreateRenderTarget();
    return true;
  }

  void CleanupRenderTarget()
  {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
  }

  void CleanupDeviceD3D()
  {
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
  }

  bool LoadTextureFromBuffer(unsigned char* buffer, int width, int height, ID3D11ShaderResourceView** out_srv, ID3D11Texture2D** out_texture)
  {
    if (buffer == nullptr) return false;

    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA* subResource = new D3D11_SUBRESOURCE_DATA();
    subResource->pSysMem = buffer;
    subResource->SysMemPitch = desc.Width * 4;
    subResource->SysMemSlicePitch = 0;

    ID3D11Texture2D* pTexture = nullptr;
    if (SUCCEEDED(g_pd3dDevice->CreateTexture2D(&desc, subResource, &pTexture)) && pTexture)
    {
      D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
      ZeroMemory(&srvDesc, sizeof(srvDesc));
      srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
      srvDesc.Texture2D.MipLevels = desc.MipLevels;
      srvDesc.Texture2D.MostDetailedMip = 0;
      if (SUCCEEDED(g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, out_srv)))
      {
        *out_texture = pTexture;
        pTexture->Release();
        return true;
      }
    }
    return false;
  }

  bool UpdateTextureBuffer(unsigned char* buffer, int width, int height, ID3D11Texture2D* in_texture)
  {
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
    int rowspan = width * 4; // 4 bytes per px
    g_pd3dDeviceContext->Map(in_texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    BYTE* mappedData = reinterpret_cast<BYTE*>(mappedResource.pData);
    for (int i = 0; i < height; ++i)
    {
      memcpy(mappedData, buffer, rowspan);
      mappedData += mappedResource.RowPitch;
      buffer += rowspan;
    }
    g_pd3dDeviceContext->Unmap(in_texture, 0);
    return true;
  }

  bool LoadTextureFromFile(const char* filename, int& out_width, int& out_height, ID3D11ShaderResourceView** out_srv, ID3D11Texture2D** out_texture)
  {
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data != NULL)
    {
      out_width = image_width;
      out_height = image_height;
      bool answer = LoadTextureFromBuffer(image_data, image_width, image_height, out_srv, out_texture);
      stbi_image_free(image_data);
      return answer;
    }
    return true;
  }
}