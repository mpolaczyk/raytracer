#include "stdafx.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>

#include "camera.h"
#include "frame_renderer.h"
#include "materials.h"
#include "bmp.h"
#include "dx11_helper.h"
#include "app.h"



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
    if (dx11::g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
    {
      dx11::CleanupRenderTarget();
      dx11::g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
      dx11::CreateRenderTarget();
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

int main(int, char**)
{
  // Create application window
  //ImGui_ImplWin32_EnableDpiAwareness();
  WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("RayTracer"), NULL };
  ::RegisterClassEx(&wc);
  HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("RayTracer"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

  // Initialize Direct3D
  if (!dx11::CreateDeviceD3D(hwnd))
  {
    dx11::CleanupDeviceD3D();
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
  ImGui_ImplDX11_Init(dx11::g_pd3dDevice, dx11::g_pd3dDeviceContext);

  // Raytracer init
  random_cache::init();
  
  app_state state;
  state.camera_setting.aspect_ratio_w = 1.0f;
  state.camera_setting.aspect_ratio_h = 1.0f;
  state.camera_setting.field_of_view = 35.0f;
  state.camera_setting.aperture = 0.02f;
  state.camera_setting.dist_to_focus = 1.0f;
  state.camera_setting.look_from = vec3(278, 278, -800);
  state.camera_setting.look_at = vec3(278, 278, 0);
  state.camera_setting.type = 0.0f;
  state.renderer_setting = renderer_config::medium_quality_preset;
  state.renderer_setting.threading_strategy = threading_strategy_type::thread_pool;
  state.renderer_setting.chunks_strategy = chunk_strategy_type::rectangles;
  state.renderer_setting.chunks_num = 200;
  state.resolution_vertical = 400;

  // Materials
  diffuse_material white_diffuse(white);
  diffuse_material green_diffuse(green);
  diffuse_material yellow_diffuse(yellow);
  diffuse_material red_diffuse(red);
  metal_material metal_shiny(grey, 0.0f);
  metal_material metal_matt(grey, 0.02f);
  dialectric_material glass(1.5f);
  solid_texture t_sky(white);
  solid_texture t_lightbulb_ultra_strong(vec3(15.0f, 15.0f, 15.0f));
  diffuse_light_material diff_light_sky = diffuse_light_material(&t_sky);
  diffuse_light_material diff_light_ultra_strong = diffuse_light_material(&t_lightbulb_ultra_strong);
  state.default_material = &metal_shiny;

  // World
  yz_rect* r1 = new yz_rect(0, 555, 0, 555, 555, &green_diffuse);
  yz_rect* r2 = new yz_rect(0, 555, 0, 555, 0, &red_diffuse);
  xz_rect* r3 = new xz_rect(213, 343, 127, 332, 554, &diff_light_ultra_strong);
  xz_rect* r4 = new xz_rect(0, 555, 0, 555, 0, &white_diffuse);
  xz_rect* r5 = new xz_rect(0, 555, 0, 555, 555, &white_diffuse);
  xy_rect* r6 = new xy_rect(0, 555, 0, 555, 555, &white_diffuse);
  sphere* e1 = new sphere(vec3(270.0f, 290.0f, 250.f), 120.f, &glass);
  sphere* e3 = new sphere(vec3(240.0f, 70.0f, 260.f), 50.f, &metal_shiny);
  sphere* e2 = new sphere(vec3(270.0f, 270.0f, 250.f), 1100.f, &diff_light_sky);
  state.world.add(r1); state.world.add(r2); state.world.add(r3); state.world.add(r4); 
  state.world.add(r5); state.world.add(r6); state.world.add(e1); state.world.add(e3); 
  state.world.add(e2);
  
  // Imgui state
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };

  raytracer_window_model rw_model;
  output_window_model ow_model;
  scene_editor_window_model sew_model;

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

    if (1)
    {
      ImGui::ShowDemoWindow();
    }

    state.world.build_boxes();  // todo, only when world dirty
    
    draw_raytracer_window(rw_model, state);
    draw_output_window(ow_model, state);
    draw_scene_editor_window(sew_model, state);

    // Rendering
    ImGui::Render();
    dx11::g_pd3dDeviceContext->OMSetRenderTargets(1, &dx11::g_mainRenderTargetView, NULL);
    dx11::g_pd3dDeviceContext->ClearRenderTargetView(dx11::g_mainRenderTargetView, clear_color_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    dx11::g_pSwapChain->Present(1, 0); // Present with vsync
    //g_pSwapChain->Present(0, 0); // Present without vsync
  }

  // Cleanup
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();

  dx11::CleanupDeviceD3D();
  ::DestroyWindow(hwnd);
  ::UnregisterClass(wc.lpszClassName, wc.hInstance);

  return 0;
}
