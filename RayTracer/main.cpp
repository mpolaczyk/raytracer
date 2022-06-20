#include "stdafx.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>

#include "camera.h"
#include "frame_renderer.h"
#include "material.h"
#include "bmp.h"
#include "dx11_helper.h"


/* 
   app_state - root structure for the application
   - accessible from multiple panels/widgets
   - holds resources
   - persistent (in future)
*/
struct app_state
{
  // Initial state
  camera_config camera_setting;
  renderer_config renderer_setting;
  hittable_list world;
  int resolution_vertical = 0;
  int resolution_horizontal = 0;
  float background_color[3] = { 0,0,0 };

  // Runtime state
  int output_width = 0;
  int output_height = 0;
  ID3D11ShaderResourceView* output_srv = nullptr;
  ID3D11Texture2D* output_texture = nullptr;
  frame_renderer renderer;
  material* default_material = nullptr;
};

/*
 *_model - for each panel/window
 - not part of app state
 - owned by UI
 - required to maintain panel/window state between frames
 - not needed to be shared between multiple panels/widgets
 - not persistent
*/
struct camera_panel_model
{
  bool use_custom_focus_distance = false;
};

struct renderer_panel_model
{

};

struct raytracer_window_model
{
  camera_panel_model cp_model;
  renderer_panel_model rp_model;
};

struct output_window_model
{
  float zoom = 1.0f;
  bool real_time_update = true;
};

struct new_object_panel_model
{
  int selected_type = 0;
  hittable* hittable = nullptr;
};

struct scene_editor_window_model
{
  int selected_id = -1;
  new_object_panel_model nop_model;
};

void draw_camera_panel(camera_panel_model& model, app_state& state);
void draw_renderer_panel(renderer_panel_model& model, app_state& state);
void draw_raytracer_window(raytracer_window_model& model, app_state& state);
void draw_output_window(output_window_model& model, app_state& state);
void draw_scene_editor_window(scene_editor_window_model& model, app_state& state);
void draw_new_object_panel(new_object_panel_model& model, app_state& state);


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

void draw_raytracer_window(raytracer_window_model& model, app_state& state)
{
  ImGui::Begin("RAYTRACER", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

  ImGuiIO& io = ImGui::GetIO();
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

  draw_camera_panel(model.cp_model, state);
  draw_renderer_panel(model.rp_model, state);
  ImGui::End();
}

void draw_camera_panel(camera_panel_model& model, app_state& state)
{
  ImGui::BeginDisabled(state.renderer.is_working());

  ImGui::Separator();
  ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "CAMERA");
  ImGui::Separator();
  int ar[2] = { state.camera_setting.aspect_ratio_w, state.camera_setting.aspect_ratio_h };
  ImGui::InputInt2("Aspect ratio", ar);
  state.camera_setting.aspect_ratio_w = ar[0];
  state.camera_setting.aspect_ratio_h = ar[1];
  ImGui::Text("Aspect ratio = %.3f", state.camera_setting.aspect_ratio_w / state.camera_setting.aspect_ratio_h);
  ImGui::InputFloat("Field of view", &state.camera_setting.field_of_view, 1.0f, 189.0f, "%.0f");
  ImGui::InputFloat("Projection", &state.camera_setting.type, 0.1f, 1.0f, "%.2f");
  ImGui::Text("0 = Perspective; 1 = Orthografic");
  ImGui::Separator();
  ImGui::InputFloat3("Look from", state.camera_setting.look_from.e, "%.2f");
  ImGui::InputFloat3("Look at", state.camera_setting.look_at.e, "%.2f");
  ImGui::Separator();
  if (model.use_custom_focus_distance)
  {
    ImGui::InputFloat("Focus distance", &state.camera_setting.dist_to_focus, 0.0f, 1000.0f, "%.2f");
  }
  else
  {
    state.camera_setting.dist_to_focus = (state.camera_setting.look_from - state.camera_setting.look_at).length();
    ImGui::Text("Focus distance = %.3f", state.camera_setting.dist_to_focus);
  }
  ImGui::Checkbox("Use custom focus distance", &model.use_custom_focus_distance);
  ImGui::InputFloat("Aperture", &state.camera_setting.aperture, 0.1f, 1.0f, "%.2f");

  ImGui::EndDisabled();
}

void draw_renderer_panel(renderer_panel_model& model, app_state& state)
{
  ImGui::BeginDisabled(state.renderer.is_working());

  ImGui::Separator();
  ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "RENDERER");
  ImGui::Separator();
  ImGui::InputInt("Resolution v", &state.resolution_vertical, 1, 2160);
  state.resolution_horizontal = (int)((float)state.resolution_vertical * state.camera_setting.aspect_ratio_w / state.camera_setting.aspect_ratio_h);
  ImGui::Text("Resolution h = %d", state.resolution_horizontal);

  ImGui::Separator();
  int chunk_strategy = (int)state.renderer_setting.chunks_strategy;
  ImGui::Combo("Chunk strategy", &chunk_strategy, chunk_strategy_names, IM_ARRAYSIZE(chunk_strategy_names));
  state.renderer_setting.chunks_strategy = (chunk_strategy_type)chunk_strategy;
  if (state.renderer_setting.chunks_strategy != chunk_strategy_type::none)
  {
    ImGui::InputInt("Chunks", &state.renderer_setting.chunks_num);
  }
  ImGui::Checkbox("Shuffle chunks", &state.renderer_setting.shuffle_chunks);

  ImGui::Separator();
  int threading_strategy = (int)state.renderer_setting.threading_strategy;
  ImGui::Combo("Threading strategy", &threading_strategy, threading_strategy_names, IM_ARRAYSIZE(threading_strategy_names));
  state.renderer_setting.threading_strategy = (threading_strategy_type)threading_strategy;
  if (state.renderer_setting.threading_strategy == threading_strategy_type::thread_pool)
  {
    ImGui::InputInt("Threads", &state.renderer_setting.threads_num);
    ImGui::Text("0 enforces std::thread::hardware_concurrency");
  }

  ImGui::Separator();
  ImGui::InputInt("Rays per pixel", &state.renderer_setting.AA_samples_per_pixel, 1, 10);
  ImGui::InputInt("Ray bounces", &state.renderer_setting.diffuse_max_bounce_num, 1);
  ImGui::InputFloat("Bounce brightness", &state.renderer_setting.diffuse_bounce_brightness, 0.01f, 0.1f, "%.2f");
  ImGui::Separator();
  ImGui::Checkbox("Enable emissive materials", &state.renderer_setting.allow_emissive);
  if (!state.renderer_setting.allow_emissive)
  {
    ImGui::ColorEdit3("Background", state.background_color, ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSidePreview);
    state.renderer_setting.background = vec3(state.background_color[0], state.background_color[1], state.background_color[2]);
  }
  ImGui::Separator();
  ImGui::Checkbox("Show time per pixel", &state.renderer_setting.pixel_time_coloring);
  if (state.renderer_setting.pixel_time_coloring)
  {
    ImGui::InputFloat("Scale", &state.renderer_setting.pixel_time_coloring_scale, 0.01f);
  }
  ImGui::Separator();

  if (ImGui::Button("Render"))
  {
    state.renderer.set_config(state.resolution_horizontal, state.resolution_vertical, state.renderer_setting, state.world, state.camera_setting);
    state.renderer.render_single_async();
    state.output_width = state.resolution_horizontal;
    state.output_height = state.resolution_vertical;
    bool ret = dx11::LoadTextureFromBuffer(state.renderer.get_img_rgb(), state.output_width, state.output_height, &state.output_srv, &state.output_texture);
    IM_ASSERT(ret);
  }
  if (state.renderer.is_working())
  {
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "WORKING");
  }
  ImGui::Text("Last render time = %lld [ms]", state.renderer.get_render_time() / 1000);
  ImGui::Text("Last save time = %lld [ms]", state.renderer.get_save_time() / 1000);

  ImGui::EndDisabled();
}

void draw_output_window(output_window_model& model, app_state& state)
{
  if (state.output_texture != nullptr)
  {
    ImGui::Begin("OUTPUT", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Checkbox("Real-time update", &model.real_time_update);
    if (!(!model.real_time_update && state.renderer.is_working()))
    {
      dx11::UpdateTextureBuffer(state.renderer.get_img_rgb(), state.output_width, state.output_height, state.output_texture);
    }
    ImGui::InputFloat("Zoom", &model.zoom, 0.1f);
    ImGui::Image((ImTextureID)state.output_srv, ImVec2(state.output_width * model.zoom, state.output_height * model.zoom), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();
  }
}

void draw_scene_editor_window(scene_editor_window_model& model, app_state& state)
{
  ImGui::Begin("SCENE", nullptr);
  ImGui::Separator();
  ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "OBJECTS");
  ImGui::Separator();

  draw_new_object_panel(model.nop_model, state);

  int num_objects = state.world.objects.size();
  if (ImGui::BeginListBox("Objects", ImVec2(-FLT_MIN, min(20, num_objects+1) * ImGui::GetTextLineHeightWithSpacing())))
  {
    for (int n = 0; n < num_objects; n++)
    {
      hittable* obj = state.world.objects[n];
      std::string obj_name;
      obj->get_name(obj_name);
      if (ImGui::Selectable(obj_name.c_str(), model.selected_id == n))
      {
        model.selected_id = n;
      }
    }
    ImGui::EndListBox();
  }

  if (model.selected_id >= 0 && model.selected_id < num_objects)
  {
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "SELECTED");
    ImGui::Separator();

    hittable* selected_obj = state.world.objects[model.selected_id];

    std::string selected_obj_name;
    selected_obj->get_name(selected_obj_name);
    ImGui::Text(selected_obj_name.c_str());

    selected_obj->draw_edit_panel();

    ImGui::Separator();
  }
  
  ImGui::End();
}

void draw_new_object_panel(new_object_panel_model& model, app_state& state)
{
  if (ImGui::Button("Add new object"))
  {
    ImGui::OpenPopup("New object?");
  }
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

  if (ImGui::BeginPopupModal("New object?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
  {
    ImGui::Combo("Object type", &model.selected_type, hittable_type_names, IM_ARRAYSIZE(hittable_type_names));
    
    if (model.hittable != nullptr && (int)model.hittable->type != model.selected_type)
    {
      delete model.hittable; // Object type changed, destroy the old one
      model.hittable = nullptr;
    }
    if (model.hittable == nullptr)
    {
      // New object
      if (model.selected_type == (int)hittable_type::hittable) { model.hittable = new hittable(); }
      else if (model.selected_type == (int)hittable_type::hittable_list) { model.hittable = new hittable_list(); }
      else if (model.selected_type == (int)hittable_type::sphere) {  model.hittable = new sphere(); }
      else if (model.selected_type == (int)hittable_type::xy_rect) { model.hittable = new xy_rect(); }
      else if (model.selected_type == (int)hittable_type::xz_rect) { model.hittable = new xz_rect(); }
      else if (model.selected_type == (int)hittable_type::yz_rect) { model.hittable = new yz_rect(); }
    }
    if (model.hittable != nullptr)
    {
      model.hittable->draw_edit_panel();
    }

    if (ImGui::Button("Add", ImVec2(120, 0)) && model.hittable != nullptr)
    {
      state.world.add(model.hittable);
      model.hittable = nullptr;
      ImGui::CloseCurrentPopup();
    }
    ImGui::SetItemDefaultFocus();
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0))) 
    { 
      if (model.hittable) 
      { 
        delete model.hittable; 
        model.hittable = nullptr;
      }
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();

  }
}