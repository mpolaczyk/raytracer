#include "stdafx.h"

#include "imgui.h"
#include "app.h"
#include "chunk_generator.h"
#include "frame_renderer.h"
#include "dx11_helper.h"

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
  if (ImGui::BeginListBox("Objects", ImVec2(-FLT_MIN, min(20, num_objects + 1) * ImGui::GetTextLineHeightWithSpacing())))
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
      else if (model.selected_type == (int)hittable_type::sphere) { model.hittable = new sphere(); }
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