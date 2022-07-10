#include "stdafx.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>

#include "app.h"
#include "dx11_helper.h"
#include "materials.h"

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
    std::cout << "Working dir: " << paths::get_working_dir().c_str() << std::endl;
    std::cout << "Project dir: " << paths::get_project_dir().c_str() << std::endl;

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
      std::string imgui_ini_filename = paths::get_imgui_file_path();
      char* buff = new char[imgui_ini_filename.size() + 1];
      strcpy(buff, imgui_ini_filename.c_str());  // returning char* is fucked up
      io.IniFilename = buff;
    }
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

    // Load persistent state
    app_state state;
    state.load_scene_state();
    state.load_rendering_state();
    state.load_window_state();
    ::SetWindowPos(hwnd, NULL, state.window.x, state.window.y, state.window.w, state.window.h, NULL);
    
    // Not yet persistent
    solid_texture*    t_white         = new solid_texture(c_white);
    solid_texture*    t_grey          = new solid_texture(c_grey);
    checker_texture*  t_checker       = new checker_texture(t_white, t_grey);
    texture_material* texture_default = new texture_material("default", t_checker);
    state.default_material = texture_default;
    state.materials.try_add(texture_default);
    
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
        if(msg.message == WM_QUIT)
        {
          done = true;
        }
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
      }
      if (done) break;
    
      // Start the Dear ImGui frame
      ImGui_ImplDX11_NewFrame();
      ImGui_ImplWin32_NewFrame();
      ImGui::NewFrame();
    
      // Draw UI
      if (0) { ImGui::ShowDemoWindow(); }
      draw_raytracer_window(state.rw_model, state);
      draw_output_window(state.ow_model, state);
      draw_scene_editor_window(state.sew_model, state);
    
      // Check if rendering is needed and do it 
      if (!state.renderer.is_working() && (state.rw_model.rp_model.render_pressed || state.ow_model.auto_render))
      {
        if (state.output_width != state.renderer_setting.resolution_horizontal || state.output_height != state.renderer_setting.resolution_vertical)
        {
          state.output_force_recreate = true;
        }
    
        bool do_render = state.rw_model.rp_model.render_pressed
          || state.output_force_recreate
          || state.renderer.is_world_dirty(state.scene_root)
          || state.renderer.is_renderer_setting_dirty(state.renderer_setting)
          || state.renderer.is_camera_setting_dirty(state.camera_setting);
    
        if (do_render)
        {
          state.scene_root.build_boxes();
          state.scene_root.update_materials(&state.materials);
    
          update_default_spawn_position(state);
    
          state.output_width = state.renderer_setting.resolution_horizontal;
          state.output_height = state.renderer_setting.resolution_vertical;
    
          state.renderer.set_config(state.renderer_setting, state.scene_root, state.camera_setting);
          state.renderer.render_single_async();
    
          if (state.output_force_recreate)
          {
            bool ret = dx11::LoadTextureFromBuffer(state.renderer.get_img_rgb(), state.output_width, state.output_height, &state.output_srv, &state.output_texture);
            IM_ASSERT(ret);
            state.output_force_recreate = false;
          }
    
          state.rw_model.rp_model.render_pressed = false;
        }
      }
    
      // Update the output panel while rendering
      if (state.renderer.is_working())
      {
        dx11::UpdateTextureBuffer(state.renderer.get_img_rgb(), state.output_width, state.output_height, state.output_texture);
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
    
      dx11::g_pSwapChain->Present(1, 0); // Present with vsync
      //g_pSwapChain->Present(0, 0); // Present without vsync
    
      RECT rect;
      ::GetWindowRect(hwnd, &rect);
      state.window.x = rect.left;
      state.window.y = rect.top;
      state.window.w = rect.right - rect.left;
      state.window.h = rect.bottom - rect.top;
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
    std::cout << "Exception handler:" << std::endl;
    std::cout << e.what() << std::endl;
    __debugbreak();
    system("pause");
  }
  return 0;
}
