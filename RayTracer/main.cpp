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
  diffuse_material* diffuse_white  = new diffuse_material("white", c_white);
  diffuse_material* diffuse_black  = new diffuse_material("black", c_black);
  diffuse_material* diffuse_red    = new diffuse_material("red", c_red);
  diffuse_material* diffuse_green  = new diffuse_material("green", c_green);
  diffuse_material* diffuse_yellow = new diffuse_material("yellow", c_yellow);
  diffuse_material* diffuse_blue   = new diffuse_material("blue", c_blue);
  state.materials.try_add(diffuse_white);
  state.materials.try_add(diffuse_black);
  state.materials.try_add(diffuse_red);
  state.materials.try_add(diffuse_green);
  state.materials.try_add(diffuse_yellow);
  state.materials.try_add(diffuse_blue);

  metal_material* metal_shiny  = new metal_material("metal_shiny", c_grey, 0.0f);
  metal_material* metal_matt   = new metal_material("metal_matt", c_grey, 0.02f);
  metal_material* metal_copper = new metal_material("copper", c_copper, 0.04f);
  metal_material* metal_steel  = new metal_material("steel", c_steel, 0.04f);
  metal_material* metal_silver = new metal_material("silver", c_silver, 0.04f);
  metal_material* metal_gold   = new metal_material("gold", c_gold, 0.04f);
  state.materials.try_add(metal_shiny);
  state.materials.try_add(metal_matt);
  state.materials.try_add(metal_copper);
  state.materials.try_add(metal_steel);
  state.materials.try_add(metal_silver);
  state.materials.try_add(metal_gold);

  dialectric_material* dialectric_water      = new dialectric_material("water", 1.33f);
  dialectric_material* dialectric_glass      = new dialectric_material("glass", 1.5f);
  dialectric_material* dialectric_sapphire   = new dialectric_material("sapphire", 1.77f);
  dialectric_material* dialectric_diamond    = new dialectric_material("diamond", 2.4f);
  dialectric_material* dialectric_moissanite = new dialectric_material("moissanite", 2.65f);  // https://en.wikipedia.org/wiki/Silicon_carbide
  state.materials.try_add(dialectric_water);
  state.materials.try_add(dialectric_glass);
  state.materials.try_add(dialectric_sapphire);
  state.materials.try_add(dialectric_diamond);
  state.materials.try_add(dialectric_moissanite);

  solid_texture*    t_white         = new solid_texture(c_white);
  solid_texture*    t_grey          = new solid_texture(c_grey);
  checker_texture*  t_checker       = new checker_texture(t_white, t_grey);
  texture_material* texture_default = new texture_material("default", t_checker);
  state.materials.try_add(texture_default);

  solid_texture* t_light         = new solid_texture(vec3(1.0f, 1.0f, 1.0f));
  solid_texture* t_light_strong  = new solid_texture(vec3(4.0f, 4.0f, 4.0f));
  solid_texture* t_light_strong2 = new solid_texture(vec3(7.0f, 7.0f, 7.0f));
  solid_texture* t_light_strong3 = new solid_texture(vec3(15.0f, 15.0f, 15.0f));
  diffuse_light_material* diff_light         = new diffuse_light_material("light", t_light);
  diffuse_light_material* diff_light_strong  = new diffuse_light_material("light_strong", t_light_strong);
  diffuse_light_material* diff_light_strong2 = new diffuse_light_material("light_strong2", t_light_strong2);
  diffuse_light_material* diff_light_strong3 = new diffuse_light_material("light_strong3", t_light_strong3);
  state.materials.try_add(diff_light);
  state.materials.try_add(diff_light_strong);
  state.materials.try_add(diff_light_strong2);
  state.materials.try_add(diff_light_strong3);

  state.default_material = texture_default;

  // World
  yz_rect* r1 = new yz_rect("green", 0, 555, 0, 555, 555);
  yz_rect* r2 = new yz_rect("red", 0, 555, 0, 555, 0);
  xz_rect* r3 = new xz_rect("light_strong3", 213, 343, 127, 332, 554);
  xz_rect* r4 = new xz_rect("white", 0, 555, 0, 555, 0);
  xz_rect* r5 = new xz_rect("white", 0, 555, 0, 555, 555);
  xy_rect* r6 = new xy_rect("white", 0, 555, 0, 555, 555);
  sphere* e1 = new sphere("glass", vec3(270.0f, 290.0f, 250.f), 120.f);
  sphere* e3 = new sphere("metal_shiny", vec3(240.0f, 70.0f, 260.f), 50.f);
  sphere* e2 = new sphere("light", vec3(270.0f, 270.0f, 250.f), 1100.f);
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

    // Draw UI
    if (0) { ImGui::ShowDemoWindow(); }
    draw_raytracer_window(rw_model, state);
    draw_output_window(ow_model, state);
    draw_scene_editor_window(sew_model, state);

    // World collisions update, TODO only when dirty
    state.world.build_boxes();
    state.world.update_materials(&state.materials);

    // Find center of the scene
    update_default_spawn_position(state);
    
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
