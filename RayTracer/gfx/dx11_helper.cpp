#include "stdafx.h"

#include <d3d11.h>
#include <d3d11_4.h>
#include <d3d11sdklayers.h>
#include <winerror.h>
#include <cstdlib>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "dx11_helper.h"

namespace dx11
{
  ID3D11Device* g_pd3dDevice = nullptr;
  ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
  IDXGISwapChain* g_pSwapChain = nullptr;
  ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
  bool g_debugLayerEnabled = false;

  namespace
  {
    bool ShouldRequestDebugLayer()
    {
      if (const char* env = std::getenv("DX11_DEBUG_LAYER"))
      {
        return env[0] != '0';
      }

#if BUILD_DEBUG
      return true;
#else
      return false;
#endif
    }

    void ConfigureDebugInterface(ID3D11Device* device)
    {
      if (device == nullptr)
      {
        return;
      }

      ID3D11InfoQueue* infoQueue = nullptr;
      if (SUCCEEDED(device->QueryInterface(__uuidof(ID3D11InfoQueue), reinterpret_cast<void**>(&infoQueue))))
      {
        infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);

        infoQueue->Release();
      }
    }

    void EnableMultithreadProtection(ID3D11DeviceContext* context)
    {
      if (context == nullptr)
      {
        return;
      }

      ID3D11Multithread* multithread = nullptr;
      if (SUCCEEDED(context->QueryInterface(__uuidof(ID3D11Multithread), reinterpret_cast<void**>(&multithread))))
      {
        multithread->SetMultithreadProtected(TRUE);
        multithread->Release();
      }
    }

    void ReportLiveObjects(ID3D11Device* device)
    {
      if (device == nullptr)
      {
        return;
      }

      ID3D11Debug* debug = nullptr;
      if (SUCCEEDED(device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debug))))
      {
        debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
        debug->Release();
      }
    }
  }

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

    bool requestDebugLayer = ShouldRequestDebugLayer();
    UINT createDeviceFlags = requestDebugLayer ? D3D11_CREATE_DEVICE_DEBUG : 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);

    if (FAILED(hr) && requestDebugLayer)
    {
      requestDebugLayer = false;
      createDeviceFlags = 0;
      hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    }

    if (FAILED(hr))
    {
      return false;
    }

    g_debugLayerEnabled = requestDebugLayer;
    if (g_debugLayerEnabled)
    {
      ConfigureDebugInterface(g_pd3dDevice);
    }

    EnableMultithreadProtection(g_pd3dDeviceContext);

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
    if (g_debugLayerEnabled && g_pd3dDevice)
    {
      ReportLiveObjects(g_pd3dDevice);
    }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    g_debugLayerEnabled = false;
  }

  bool LoadTextureFromBuffer(unsigned char* buffer, int width, int height, ID3D11ShaderResourceView** out_srv, ID3D11Texture2D** out_texture)
  {
    if (buffer == nullptr || out_srv == nullptr || out_texture == nullptr || g_pd3dDevice == nullptr) return false;

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

    D3D11_SUBRESOURCE_DATA subResource;
    subResource.pSysMem = buffer;
    subResource.SysMemPitch = desc.Width * 4;
    subResource.SysMemSlicePitch = 0;

    ID3D11Texture2D* pTexture = nullptr;
    if (SUCCEEDED(g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture)) && pTexture)
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
        return true;
      }
      pTexture->Release();
    }
    return false;
  }

  bool UpdateTextureBuffer(unsigned char* buffer, int width, int height, ID3D11Texture2D* in_texture)
  {
    if (buffer == nullptr || in_texture == nullptr || g_pd3dDeviceContext == nullptr) return false;

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
    int rowspan = width * 4; // 4 bytes per px
    if (FAILED(g_pd3dDeviceContext->Map(in_texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
    {
      return false;
    }
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
    return false;
  }

  bool InitializeGpuTimer(gpu_timer& timer)
  {
    if (g_pd3dDevice == nullptr)
    {
      return false;
    }

    if (timer.disjoint_query == nullptr)
    {
      D3D11_QUERY_DESC desc{};
      desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
      if (FAILED(g_pd3dDevice->CreateQuery(&desc, &timer.disjoint_query)))
      {
        return false;
      }
    }

    if (timer.start_query == nullptr)
    {
      D3D11_QUERY_DESC desc{};
      desc.Query = D3D11_QUERY_TIMESTAMP;
      if (FAILED(g_pd3dDevice->CreateQuery(&desc, &timer.start_query)))
      {
        return false;
      }
    }

    if (timer.end_query == nullptr)
    {
      D3D11_QUERY_DESC desc{};
      desc.Query = D3D11_QUERY_TIMESTAMP;
      if (FAILED(g_pd3dDevice->CreateQuery(&desc, &timer.end_query)))
      {
        return false;
      }
    }

    return true;
  }

  void BeginGpuTimer(gpu_timer& timer)
  {
    if (g_pd3dDeviceContext == nullptr || timer.disjoint_query == nullptr || timer.start_query == nullptr)
    {
      return;
    }

    g_pd3dDeviceContext->Begin(timer.disjoint_query);
    g_pd3dDeviceContext->End(timer.start_query);
  }

  void EndGpuTimer(gpu_timer& timer)
  {
    if (g_pd3dDeviceContext == nullptr || timer.disjoint_query == nullptr || timer.end_query == nullptr)
    {
      return;
    }

    g_pd3dDeviceContext->End(timer.end_query);
    g_pd3dDeviceContext->End(timer.disjoint_query);
  }

  bool ReadGpuTimeMs(gpu_timer& timer, double& out_ms)
  {
    out_ms = 0.0;
    if (g_pd3dDeviceContext == nullptr || timer.disjoint_query == nullptr || timer.start_query == nullptr || timer.end_query == nullptr)
    {
      return false;
    }

    D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjoint_data{};
    while (g_pd3dDeviceContext->GetData(timer.disjoint_query, &disjoint_data, sizeof(disjoint_data), 0) == S_FALSE)
    {
    }

    if (disjoint_data.Disjoint || disjoint_data.Frequency == 0)
    {
      return false;
    }

    UINT64 start_time = 0;
    UINT64 end_time = 0;
    while (g_pd3dDeviceContext->GetData(timer.start_query, &start_time, sizeof(start_time), 0) == S_FALSE)
    {
    }
    while (g_pd3dDeviceContext->GetData(timer.end_query, &end_time, sizeof(end_time), 0) == S_FALSE)
    {
    }

    out_ms = (static_cast<double>(end_time - start_time) / static_cast<double>(disjoint_data.Frequency)) * 1000.0;
    return true;
  }

  void ReleaseGpuTimer(gpu_timer& timer)
  {
    if (timer.disjoint_query) { timer.disjoint_query->Release(); timer.disjoint_query = nullptr; }
    if (timer.start_query) { timer.start_query->Release(); timer.start_query = nullptr; }
    if (timer.end_query) { timer.end_query->Release(); timer.end_query = nullptr; }
  }
}