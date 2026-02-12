#include "stdafx.h"

#include "windows_minimal.h"

#include <thread>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>

#include "app/app.h"
#include "gfx/dx11_helper.h"
#include "math/materials.h"
#include "processing/async_renderer_base.h"
#include "renderers/gpu_reference_renderer.h"
#include "math/fpexcept.h"
#include "app/factories.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern void seh_exception_handler(unsigned int u, _EXCEPTION_POINTERS* pExp);

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
#if USE_FPEXCEPT
  if (!IsDebuggerPresent())
  {
    // Register SEH exception catching when no debugger is present
    _set_se_translator(seh_exception_handler);
  }
  fpexcept::enabled_scope fpe;
#endif

  try
  {
    logger::info("Working dir: {0}", io::get_working_dir());
    logger::info("Workspace dir: {0}", io::get_workspace_dir());

    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("RayTracer"), NULL };
    ::RegisterClassEx(&wc);
    HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("RayTracer"), WS_OVERLAPPEDWINDOW, 100, 100, 1920, 1080, NULL, NULL, wc.hInstance, NULL);
    
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
    {
      // overwrite imgui config file name
      std::string imgui_ini_filename = io::get_imgui_file_path();
      char* buff = new char[imgui_ini_filename.size() + 1];
      strcpy(buff, imgui_ini_filename.c_str());  // returning char* is fucked up
      io.IniFilename = buff;
    }
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();
    ImGui::GetIO().KeyRepeatDelay = 0.1f;
    //ImGui::GetIO().KeyRepeatRate = 0.01f;

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(dx11::g_pd3dDevice, dx11::g_pd3dDeviceContext);
    
    // Raytracer init
    random_cache::init();

    // Load persistent state
    app_instance state;
    state.load_scene_state();
    state.load_rendering_state();
    state.load_window_state();
    state.scene_root->load_resources();
    state.renderer = object_factory::spawn_renderer(state.renderer_conf->type);
    ::SetWindowPos(hwnd, NULL, state.window_conf.x, state.window_conf.y, state.window_conf.w, state.window_conf.h, NULL);

    // Auto render on startup
    state.rw_model.rp_model.render_pressed = true;

    // Main loop
    while (state.is_running)
    {
      // Poll and handle messages (inputs, window resize, etc.)
      // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
      // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
      // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
      // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
      MSG msg;
      while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
      {
        if(msg.message == WM_QUIT)
        {
          state.is_running = false;
        }
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
      }
      if (!state.is_running) break;
    
      // Start the Dear ImGui frame
      ImGui_ImplDX11_NewFrame();
      ImGui_ImplWin32_NewFrame();
      ImGui::NewFrame();

      state.reload_scene_state_if_changed();
      handle_input(state);

      // Draw UI
#ifndef IMGUI_DISABLE_DEMO_WINDOWS
      // Debug UI only in debug mode
      if (0) { ImGui::ShowDemoWindow(); }
#endif
      draw_raytracer_window(state.rw_model, state);
      draw_output_window(state.ow_model, state);
      draw_scene_editor_window(state.sew_model, state);

      // Check if rendering is needed and do it 
      if (state.renderer != nullptr)
      {
        const bool force_frame = state.renderer->wants_sync_render();
        const bool is_working = state.renderer->is_working();
        if (force_frame || !is_working && (state.rw_model.rp_model.render_pressed || state.ow_model.auto_render))
        {
          const bool do_render = state.rw_model.rp_model.render_pressed
            || state.renderer->is_world_dirty(state.scene_root)
            || state.renderer->is_renderer_setting_dirty(state.renderer_conf)
            || state.renderer->is_camera_setting_dirty(state.camera_conf);
          
          if (force_frame || do_render)
          {
            if (state.renderer->is_renderer_type_different(state.renderer_conf))
            {
              delete state.renderer;
              state.output_srv = nullptr;
              state.output_texture = nullptr;
              state.renderer = object_factory::spawn_renderer(state.renderer_conf->type);
            }

            state.scene_root->load_resources();
            state.scene_root->pre_render();
            state.scene_root->build_boxes();
            state.scene_root->update_materials(state.materials);
            state.scene_root->query_lights();

            update_default_spawn_position(state);

            state.output_width = state.renderer_conf->resolution_horizontal;
            state.output_height = state.renderer_conf->resolution_vertical;

            state.renderer->set_config(state.renderer_conf, state.scene_root, state.camera_conf);
            if (force_frame)
            {
              state.renderer->render_single_sync();
            }
            else
            {
              state.renderer->render_single_async();
            }

            if (state.renderer->get_renderer_type() == renderer_type::gpu_reference)
            {
              auto* gpu_renderer = static_cast<gpu_reference_renderer*>(state.renderer);
              ID3D11ShaderResourceView* renderer_srv = gpu_renderer->get_output_srv();
              ID3D11Texture2D* renderer_texture = gpu_renderer->get_output_texture();
              if (renderer_srv != nullptr && renderer_texture != nullptr)
              {
                state.output_srv = renderer_srv;
                state.output_texture = renderer_texture;
              }
            }
            if (state.output_texture == nullptr)
            {
              const bool ret = dx11::LoadTextureFromBuffer(state.renderer->get_img_rgb(), state.output_width, state.output_height, &state.output_srv, &state.output_texture);
              IM_ASSERT(ret);
            }

            state.rw_model.rp_model.render_pressed = false;
          }
        }

        // Update the output panel
        if (state.output_texture)
        {
          if (state.renderer->get_renderer_type() == renderer_type::gpu_reference)
          {
            auto* gpu_renderer = static_cast<gpu_reference_renderer*>(state.renderer);
            ID3D11ShaderResourceView* renderer_srv = gpu_renderer->get_output_srv();
            ID3D11Texture2D* renderer_texture = gpu_renderer->get_output_texture();
            if (renderer_srv != nullptr && renderer_texture != nullptr)
            {
              state.output_srv = renderer_srv;
              state.output_texture = renderer_texture;
            }
            else
            {
              dx11::UpdateTextureBuffer(state.renderer->get_img_rgb(), state.output_width, state.output_height, state.output_texture);
            }
          }
          else
          {
            dx11::UpdateTextureBuffer(state.renderer->get_img_rgb(), state.output_width, state.output_height, state.output_texture);
          }
        }
      }
            
      // UI rendering
      static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
      static float clear_color_with_alpha[4] = 
      { 
        clear_color.x * clear_color.w, 
        clear_color.y * clear_color.w, 
        clear_color.z * clear_color.w, 
        clear_color.w 
      };
      ImGui::Render();
      dx11::g_pd3dDeviceContext->OMSetRenderTargets(1, &dx11::g_mainRenderTargetView, NULL);
      dx11::g_pd3dDeviceContext->ClearRenderTargetView(dx11::g_mainRenderTargetView, clear_color_with_alpha);
      ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    
      if (true)
      {
        dx11::g_pSwapChain->Present(1, 0); // Present with vsync
      }
      else
      {
        // Hardcoded frame limiter
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        dx11::g_pSwapChain->Present(0, 0); // Present without vsync
      }
    
      RECT rect;
      ::GetWindowRect(hwnd, &rect);
      state.window_conf.x = rect.left;
      state.window_conf.y = rect.top;
      state.window_conf.w = rect.right - rect.left;
      state.window_conf.h = rect.bottom - rect.top;
    }
    
    state.save_window_state();
    
    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    
    dx11::CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);
  }
  catch (const std::exception& e)
  {
    logger::critical("Exception handler:");
    logger::critical("{0}", e.what());
    __debugbreak();
    system("pause");
  }
  return 0;
}
