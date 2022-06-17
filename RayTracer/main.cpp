#include "stdafx.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "camera.h"
#include "frame_renderer.h"
#include "material.h"
#include "bmp.h"

// Data
static ID3D11Device* g_pd3dDevice = NULL;
static ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
static IDXGISwapChain* g_pSwapChain = NULL;
static ID3D11RenderTargetView* g_mainRenderTargetView = NULL;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool LoadTextureFromFile(const char* filename, ID3D11ShaderResourceView** out_srv, int& out_width, int& out_height);
bool LoadTextureFromBuffer(unsigned char* buffer, ID3D11ShaderResourceView** out_srv, int width, int height);

// Main code
int main(int, char**)
{
  // Create application window
  //ImGui_ImplWin32_EnableDpiAwareness();
  WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("RayTracer"), NULL };
  ::RegisterClassEx(&wc);
  HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("RayTracer"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

  // Initialize Direct3D
  if (!CreateDeviceD3D(hwnd))
  {
    CleanupDeviceD3D();
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);
    return 1;
  }

  // Show the window
  ::ShowWindow(hwnd, SW_SHOWDEFAULT);
  ::UpdateWindow(hwnd);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  //ImGui::StyleColorsClassic();

  // Setup Platform/Renderer backends
  ImGui_ImplWin32_Init(hwnd);
  ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

  random_cache::init();

  // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
  // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
  // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
  // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
  // - Read 'docs/FONTS.md' for more instructions and details.
  // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
  //io.Fonts->AddFontDefault();
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
  //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
  //IM_ASSERT(font != NULL);

  // Imgui state
  bool show_demo_window = true;
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  // Camera
  int ar[2] = { 1, 1 };
  bool use_custom_focus_distance = false;
  camera_config camera_setting;
  camera_setting.aspect_ratio = (float)ar[0] / (float)ar[1];
  camera_setting.field_of_view = 30.0f;
  camera_setting.aperture = 0.02f;
  camera_setting.dist_to_focus = 1.0f;
  camera_setting.look_from = vec3(278, 278, -800);
  camera_setting.look_at = vec3(278, 278, 0);
  camera_setting.type = 0.0f;

  // Renderer
  renderer_config renderer_setting = renderer_config::high_quality_preset;
  int resolution_vertical = 400;
  int resolution_horizontal = (int)((float)resolution_vertical * camera_setting.aspect_ratio);
  int chunk_strategy = (int)renderer_setting.chunks_strategy;
  int threading_strategy = (int)renderer_setting.threading_strategy;
  bool is_rendering = false;
  frame_renderer renderer;

  // Materials
  diffuse_material white_diffuse(white);
  diffuse_material green_diffuse(green);
  diffuse_material yellow_diffuse(yellow);
  diffuse_material red_diffuse(red);
  metal_material metal_shiny(grey, 0.0f);
  metal_material metal_matt(grey, 0.02f);
  dialectric_material glass(1.5f);
  solid_texture t_sky(white * 0.4f);
  solid_texture t_lightbulb_ultra_strong(vec3(15.0f, 15.0f, 15.0f));
  diffuse_light_material diff_light_sky = diffuse_light_material(&t_sky);
  diffuse_light_material diff_light_ultra_strong = diffuse_light_material(&t_lightbulb_ultra_strong);

  // World
  hittable_list world;
  yz_rect* r1 = new yz_rect(0, 555, 0, 555, 555, &green_diffuse);
  yz_rect* r2 = new yz_rect(0, 555, 0, 555, 0, &red_diffuse);
  xz_rect* r3 = new xz_rect(213, 343, 127, 332, 554, &diff_light_ultra_strong);
  xz_rect* r4 = new xz_rect(0, 555, 0, 555, 0, &white_diffuse);
  xz_rect* r5 = new xz_rect(0, 555, 0, 555, 555, &white_diffuse);
  xy_rect* r6 = new xy_rect(0, 555, 0, 555, 555, &white_diffuse);
  sphere* e1 = new sphere(vec3(230.0f, 290.0f, 250.f), 120.f, &glass);
  sphere* e3 = new sphere(vec3(270.0f, 50.0f, 210.f), 30.f, &metal_shiny);
  sphere* e2 = new sphere(vec3(270.0f, 270.0f, 250.f), 1100.f, &diff_light_sky);
  world.add(r1); world.add(r2); world.add(r3); world.add(r4); world.add(r5); world.add(r6);   world.add(e1); world.add(e3); world.add(e2);
  world.build_boxes();

  // Render 
  int my_image_width = 0;
  int my_image_height = 0;
  ID3D11ShaderResourceView* my_texture = nullptr;

  if (1)
  {
    bool ret = LoadTextureFromFile("image_0.bmp", &my_texture, my_image_width, my_image_height);
    IM_ASSERT(ret);
  }
  else 
  {
    renderer.set_config(resolution_horizontal, resolution_vertical, renderer_setting);
    renderer.render_single(world, camera_setting);
    my_image_width = renderer.image_width;
    my_image_height = renderer.image_height;
    bool ret = LoadTextureFromBuffer(renderer.img->buffer, &my_texture, renderer.image_width, renderer.image_height);
    IM_ASSERT(ret);
  }

  // Main loop
  bool done = false;
  while (!done)
  {
    // Poll and handle messages (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    MSG msg;
    while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
    {
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
      if (msg.message == WM_QUIT)
        done = true;
    }
    if (done)
      break;

    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (show_demo_window)
    {
      ImGui::ShowDemoWindow(&show_demo_window);
    }
    
    {
      ImGui::Begin("RayTracer", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

      ImGui::Separator();
      ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "CAMERA");
      ImGui::Separator();
      ImGui::InputInt2("Aspect ratio", ar);
      camera_setting.aspect_ratio = (float)ar[0] / (float)ar[1];
      ImGui::Text("Aspect ratio = %.3f", camera_setting.aspect_ratio);
      ImGui::InputFloat("Field of view", &camera_setting.field_of_view, 1.0f, 189.0f, "%.0f");
      ImGui::InputFloat("Projection", &camera_setting.type, 0.1f, 1.0f, "%.2f");
      ImGui::Text("0 = Orthografic; 1 = Perspective");
      ImGui::Separator();
      ImGui::InputFloat3("Look from", camera_setting.look_from.e, "%.2f");
      ImGui::InputFloat3("Look at", camera_setting.look_at.e, "%.2f");
      ImGui::Separator();
      if (use_custom_focus_distance)
      {
        ImGui::InputFloat("Focus distance", &camera_setting.dist_to_focus, 0.0f, 1000.0f, "%.2f");
      }
      else
      {
        camera_setting.dist_to_focus = (camera_setting.look_from - camera_setting.look_at).length();
        ImGui::Text("Focus distance = %.3f", camera_setting.dist_to_focus);
      }
      ImGui::Checkbox("Use custom focus distance", &use_custom_focus_distance);
      ImGui::InputFloat("Aperture", &camera_setting.aperture, 0.1f, 1.0f, "%.2f");

      ImGui::Separator();
      ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "RENDERER");
      ImGui::Separator();
      ImGui::InputInt("Resolution v", &resolution_vertical, 1, 2160);
      resolution_horizontal = (int)((float)resolution_vertical * camera_setting.aspect_ratio);
      ImGui::Text("Resolution h = %d", resolution_horizontal);
      ImGui::Separator();
      ImGui::Combo("Chunk strategy", &chunk_strategy, chunk_strategy_names, IM_ARRAYSIZE(chunk_strategy_names));
      renderer_setting.chunks_strategy = (chunk_strategy_type)chunk_strategy;
      if (chunk_strategy != (int)chunk_strategy_type::none)
      {
        ImGui::InputInt("Chunks", &renderer_setting.chunks_num);
      }
      ImGui::Separator();
      ImGui::Combo("Threading strategy", &threading_strategy, threading_strategy_names, IM_ARRAYSIZE(threading_strategy_names));
      renderer_setting.threading_strategy = (threading_strategy_type)threading_strategy;
      if (threading_strategy == (int)threading_strategy_type::thread_pool)
      {
        ImGui::InputInt("Threads", &renderer_setting.threads_num);
        ImGui::Text("0 enforces std::thread::hardware_concurrency");
      }
      ImGui::Separator();
      ImGui::InputInt("Rays per pixel", &renderer_setting.AA_samples_per_pixel,1,10);
      ImGui::Separator();
      ImGui::InputInt("Ray bounces", &renderer_setting.diffuse_max_bounce_num, 1);
      ImGui::InputFloat("Bounce brightness", &renderer_setting.diffuse_bounce_brightness, 0.01f, 0.1f, "%.2f");
      ImGui::Separator();
      
      if (ImGui::Button("Render"))
      {
        renderer.set_config(resolution_horizontal, resolution_vertical, renderer_setting);
        renderer.render_single(world, camera_setting);
        my_image_width = renderer.image_width;
        my_image_height = renderer.image_height;
        //bool ret = LoadTextureFromBuffer(renderer.img->buffer, &my_texture, renderer.image_width, renderer.image_height);
        //IM_ASSERT(ret);
        bool ret = LoadTextureFromFile("image_0.bmp", &my_texture, my_image_width, my_image_height);
        IM_ASSERT(ret);
      }
      ImGui::End();

      if (my_texture != nullptr)
      {
        ImGui::Begin("Output", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Image((ImTextureID)my_texture, ImVec2(my_image_width, my_image_height));
        ImGui::End();
      }

    }

    // Rendering
    ImGui::Render();
    const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    g_pSwapChain->Present(1, 0); // Present with vsync
    //g_pSwapChain->Present(0, 0); // Present without vsync
  }

  // Cleanup
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();

  CleanupDeviceD3D();
  ::DestroyWindow(hwnd);
  ::UnregisterClass(wc.lpszClassName, wc.hInstance);

  return 0;
}


bool LoadTextureFromBuffer(unsigned char* buffer, ID3D11ShaderResourceView** out_srv, int width, int height)
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
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  desc.CPUAccessFlags = 0;

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
      pTexture->Release();
      return true;
    }
  }
  return false;
}

bool LoadTextureFromFile(const char* filename, ID3D11ShaderResourceView** out_srv, int& out_width, int& out_height)
{
  int image_width = 0;
  int image_height = 0;
  unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
  if (image_data != NULL)
  {
    out_width = image_width;
    out_height = image_height;

    bool answer = LoadTextureFromBuffer(image_data, out_srv, image_width, image_height);
    stbi_image_free(image_data);
    return answer;
  }
  return true;
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

void CleanupDeviceD3D()
{
  CleanupRenderTarget();
  if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
  if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
  if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
  ID3D11Texture2D* pBackBuffer;
  g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
  g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
  pBackBuffer->Release();
}

void CleanupRenderTarget()
{
  if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    return true;

  switch (msg)
  {
  case WM_SIZE:
    if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
    {
      CleanupRenderTarget();
      g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
      CreateRenderTarget();
    }
    return 0;
  case WM_SYSCOMMAND:
    if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
      return 0;
    break;
  case WM_DESTROY:
    ::PostQuitMessage(0);
    return 0;
  }
  return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

