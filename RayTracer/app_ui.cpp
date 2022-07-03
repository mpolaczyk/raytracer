#include "stdafx.h"

#include "imgui.h"
#include "app.h"
#include "chunk_generator.h"
#include "frame_renderer.h"

void draw_raytracer_window(raytracer_window_model& model, app_state& state)
{
  ImGui::Begin("RAYTRACER", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

  if (ImGui::MenuItem("SAVE STATE"))
  {
    state.save_rendering_state();
  }
  ImGui::Separator();

  ImGuiIO& io = ImGui::GetIO();
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

  draw_renderer_panel(model.rp_model, state);
  ImGui::End();
}

void draw_camera_panel(camera_panel_model& model, app_state& state)
{
  ImGui::Separator();
  ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "CAMERA");
  ImGui::Separator();
  float ar[2] = { state.camera_setting.aspect_ratio_w, state.camera_setting.aspect_ratio_h };
  ImGui::InputFloat2("Aspect ratio", ar);
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
}

void draw_renderer_panel(renderer_panel_model& model, app_state& state)
{
  ImGui::Separator();
  ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "RENDERER");
  ImGui::Separator();
  ImGui::InputInt("Resolution v", &state.renderer_setting.resolution_vertical, 1, 2160);
  state.renderer_setting.resolution_horizontal = (int)((float)state.renderer_setting.resolution_vertical * state.camera_setting.aspect_ratio_w / state.camera_setting.aspect_ratio_h);
  ImGui::Text("Resolution h = %d", state.renderer_setting.resolution_horizontal);

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
    ImGui::ColorEdit3("Background", model.background_color, ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSidePreview);
    state.renderer_setting.background = vec3(model.background_color[0], model.background_color[1], model.background_color[2]);
  }
  ImGui::Separator();
  ImGui::Checkbox("Show time per pixel", &state.renderer_setting.pixel_time_coloring);
  if (state.renderer_setting.pixel_time_coloring)
  {
    ImGui::InputFloat("Scale", &state.renderer_setting.pixel_time_coloring_scale, 0.01f);
  }
  ImGui::Separator();

  ImGui::Checkbox("Reuse buffers", &state.renderer_setting.reuse_buffer);
  if (ImGui::Button("Render"))
  {
    model.render_pressed = true;
  }
  if (state.renderer.is_working())
  {
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "WORKING");
  }
  ImGui::Text("Last render time = %lld [ms]", state.renderer.get_render_time() / 1000);
  ImGui::Text("Last save time = %lld [ms]", state.renderer.get_save_time() / 1000);
}

void draw_output_window(output_window_model& model, app_state& state)
{
  if (state.output_texture != nullptr)
  {
    ImGui::Begin("OUTPUT", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::InputFloat("Zoom", &model.zoom, 0.1f);
    ImGui::Image((ImTextureID)state.output_srv, ImVec2(state.output_width * model.zoom, state.output_height * model.zoom), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::Checkbox("Auto render on scene change", &model.auto_render);
    ImGui::End();
  }
}

void draw_scene_editor_window(scene_editor_window_model& model, app_state& state)
{
  ImGui::Begin("SCENE", nullptr);
  
  if (ImGui::MenuItem("SAVE STATE"))
  {
    state.save_scene_state();
  }

  draw_camera_panel(model.cp_model, state);

  ImGui::Separator();
  ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "OBJECTS");
  ImGui::Separator();

  draw_new_object_panel(model.nop_model, state);
  ImGui::SameLine();
  draw_delete_object_panel(model.d_model, state);

  int num_objects = (int)state.scene_root.objects.size();
  if (ImGui::BeginListBox("Objects", ImVec2(-FLT_MIN, min(20, num_objects + 1) * ImGui::GetTextLineHeightWithSpacing())))
  {
    for (int n = 0; n < num_objects; n++)
    {
      hittable* obj = state.scene_root.objects[n];
      std::string obj_name;
      obj->get_name(obj_name, false);
      if (ImGui::Selectable(obj_name.c_str(), model.selected_id == n))
      {
        model.m_model.selected_material_name_index = -1;
        model.selected_id = n;
        model.d_model.selected_id = n;
      }
    }
    ImGui::EndListBox();
  }

  if (model.selected_id >= 0 && model.selected_id < num_objects)
  {
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "SELECTED");
    ImGui::Separator();

    hittable* selected_obj = state.scene_root.objects[model.selected_id];
    selected_obj->draw_edit_panel();

    material_selection_combo_model m_model;
    if (model.m_model.selected_material_name_index == -1)
    {
      m_model.selected_material_name_index = state.materials.get_index_by_id(selected_obj->material_id);
    }
    draw_material_selection_combo(m_model, state);
    selected_obj->material_id = state.materials.get_material_ids()[m_model.selected_material_name_index];

    ImGui::Separator();
  }
  ImGui::End();
}

void draw_new_object_panel(new_object_panel_model& model, app_state& state)
{
  if (ImGui::Button("Add new"))
  {
    ImGui::OpenPopup("New object?");
  }
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

  if (ImGui::BeginPopupModal("New object?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
  {
    ImGui::Combo("Object type", &model.selected_type, hittable_class_names, IM_ARRAYSIZE(hittable_class_names));
    ImGui::Separator();

    if (model.hittable != nullptr && (int)model.hittable->type != model.selected_type)
    {
      delete model.hittable; // Object type changed, destroy the old one
      model.hittable = nullptr;
    }
    if (model.hittable == nullptr)
    {
      // New object
      model.hittable = hittable::spawn_by_type((hittable_class)model.selected_type);
      model.hittable->set_origin(state.center_of_scene);
    }

    if (model.hittable != nullptr)
    {
      model.hittable->draw_edit_panel();

      draw_material_selection_combo(model.m_model, state);
    }

    if (ImGui::Button("Add", ImVec2(120, 0)) && model.hittable != nullptr)
    {
      model.hittable->material_id = state.materials.get_material_ids()[model.m_model.selected_material_name_index];
      state.scene_root.add(model.hittable);
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

void draw_material_selection_combo(material_selection_combo_model& model, app_state& state)
{
  std::vector<std::string> material_names = state.materials.get_material_names();
  if (material_names.size() > 0)
  {
    ImGui::Separator();
    if (ImGui::BeginCombo("Material", material_names[model.selected_material_name_index].c_str()))
    {
      for (int i = 0; i < material_names.size(); ++i)
      {
        const bool isSelected = (model.selected_material_name_index == i);
        if (ImGui::Selectable(material_names[i].c_str(), isSelected))
        {
          model.selected_material_name_index = i;
        }
        if (isSelected)
        {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }
  }
  else
  {
    ImGui::Text("No materials to choose from");
  }
}

void draw_delete_object_panel(delete_object_panel_model& model, app_state& state)
{
  if (ImGui::Button("Delete selected"))
  {
    ImGui::OpenPopup("Delete object?");
  }
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

  if (ImGui::BeginPopupModal("Delete object?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
  {
    hittable* selected_obj = state.scene_root.objects[model.selected_id];
    if (selected_obj != nullptr)
    {
      ImGui::BeginDisabled(true);
      selected_obj->draw_edit_panel();
      ImGui::EndDisabled();

      if (ImGui::Button("Delete", ImVec2(120, 0)))
      {
        state.scene_root.remove(model.selected_id);
        ImGui::CloseCurrentPopup();
      }
      ImGui::SetItemDefaultFocus();
      ImGui::SameLine();
      if (ImGui::Button("Cancel", ImVec2(120, 0)))
      {
        ImGui::CloseCurrentPopup();
      }
    }
    ImGui::EndPopup();
  }
}