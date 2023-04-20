#include "stdafx.h"

#include "app.h"
#include "imgui.h"

void update_default_spawn_position(app_instance& state)
{
  // Find center of the scene, new objects scan be spawned there
  vec3 look_from = state.camera_conf.look_from;
  vec3 look_dir = state.camera_conf.look_dir;
  float dist_to_focus = state.camera_conf.dist_to_focus;

  // Ray to the look at position to find non colliding spawn point
  ray center_of_scene_ray(look_from, look_dir);
  hit_record center_of_scene_hit;
  if (state.scene_root.hit(center_of_scene_ray, 0.01f, 2.0f*dist_to_focus, center_of_scene_hit))
  {
    state.center_of_scene = center_of_scene_hit.p;
    state.distance_to_center_of_scene = (center_of_scene_hit.p - look_from).length();
  }
  else
  {
    state.center_of_scene = look_from - look_dir * dist_to_focus;
    state.distance_to_center_of_scene = dist_to_focus;
  }
}

void handle_input(app_instance& state)
{
  // Handle clicks on the output window - select the object under the cursor
  if (state.output_window_lmb_x > 0.0f && state.output_window_lmb_y > 0.0f)
  {
    float u = state.output_window_lmb_x / (state.output_width - 1);
    float v = state.output_window_lmb_y / (state.output_height - 1);
    v = 1.0f - v; // because vertical axis is flipped in the output window
    camera cam;
    cam.set_camera(state.camera_conf);
    ray r = cam.get_ray(u, v);
    hit_record hit;
    if (state.scene_root.hit(r, 0.0f, infinity, hit))
    {
      state.selected_object = hit.object;
    }

    state.output_window_lmb_x = -1.0f;
    state.output_window_lmb_y = -1.0f;
  }

  // Handle hotkeys
  if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
  {
    state.is_running = false;
  }
  if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F1)))
  {
    state.renderer_conf.type = renderer_type::preview;
  }
  if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F2)))
  {
    state.renderer_conf.type = renderer_type::reference;
  }
  if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F5)))
  {
    state.rw_model.rp_model.render_pressed = true;
  }

  // Handle speed
  float wheel_delta = ImGui::GetIO().MouseWheel;
  state.camera_move_speed = max(0.5f, state.camera_move_speed + wheel_delta / 2.0f);

  // Handle camera movement
  if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_E)))
  {
    state.camera_conf.move_up(state.camera_move_speed);
  }
  if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Q)))
  {
    state.camera_conf.move_down(state.camera_move_speed);
  }
  if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_W)))
  {
    state.camera_conf.move_forward(state.camera_move_speed);
  }
  if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S)))
  {
    state.camera_conf.move_backward(state.camera_move_speed);
  }
  if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_A)))
  {
    state.camera_conf.move_left(state.camera_move_speed);
  }
  if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_D)))
  {
    state.camera_conf.move_right(state.camera_move_speed);
  }
  
  // Handle camera rotation
  if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
  {
    ImVec2 mouse_delta = ImGui::GetIO().MouseDelta;

    // Check if the mouse has moved
    if (mouse_delta.x != 0.0f || mouse_delta.y != 0.0f)
    {
      float rotate_speed = 0.001f; // proportion - screen space delta to radians
      state.camera_conf.rotate(mouse_delta.x * rotate_speed, mouse_delta.y * rotate_speed);
    }
  }
 
}