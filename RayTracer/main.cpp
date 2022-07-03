#include "stdafx.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>

#include "app.h"
#include "dx11_helper.h"
#include "materials.h"


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
  
  // Load state
  app_state state;
  load_app_state(state);

  // Not yet persistent
  solid_texture*    t_white         = new solid_texture(c_white);
  solid_texture*    t_grey          = new solid_texture(c_grey);
  checker_texture*  t_checker       = new checker_texture(t_white, t_grey);
  texture_material* texture_default = new texture_material("default", t_checker);
  state.default_material = texture_default;
  
  // Imgui state
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };

  // Window models
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

    // Draw UI
    if (0) { ImGui::ShowDemoWindow(); }
    draw_raytracer_window(rw_model, state);
    draw_output_window(ow_model, state);
    draw_scene_editor_window(sew_model, state);

    // Check if rendering is needed and do it 
    if (!state.renderer.is_working() && (rw_model.rp_model.render_pressed || ow_model.auto_refresh))
    {
      if (state.output_width != state.resolution_horizontal || state.output_height != state.resolution_vertical)
      {
        state.output_force_recreate = true;
      }

      bool do_render = rw_model.rp_model.render_pressed 
        || state.output_force_recreate
        || state.renderer.is_world_dirty(state.world)
        || state.renderer.is_renderer_setting_dirty(state.renderer_setting)
        || state.renderer.is_camera_setting_dirty(state.camera_setting);

      if (do_render)
      {
        state.world.build_boxes();
        state.world.update_materials(&state.materials);

        update_default_spawn_position(state);

        state.output_width = state.resolution_horizontal;
        state.output_height = state.resolution_vertical;

        state.renderer.set_config(state.resolution_horizontal, state.resolution_vertical, state.renderer_setting, state.world, state.camera_setting);
        state.renderer.render_single_async();

        if (state.output_force_recreate)
        {
          bool ret = dx11::LoadTextureFromBuffer(state.renderer.get_img_rgb(), state.output_width, state.output_height, &state.output_srv, &state.output_texture);
          IM_ASSERT(ret);
          state.output_force_recreate = false;
        }

        rw_model.rp_model.render_pressed = false;
      }
    }

    // Update the output panel while rendering
    if (state.renderer.is_working())
    {
      dx11::UpdateTextureBuffer(state.renderer.get_img_rgb(), state.output_width, state.output_height, state.output_texture);
    }
    
    // UI rendering
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
