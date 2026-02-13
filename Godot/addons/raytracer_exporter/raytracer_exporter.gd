@tool
extends EditorPlugin

const EXPORT_PATH_SETTING := "raytracer/export_path"
const DEFAULT_EXPORT_PATH := "res://scene.json"

const TYPE_SCENE := 0
const TYPE_SPHERE := 1
const TYPE_STATIC_MESH := 5

const EXPORT_CHECK_INTERVAL_MSEC := 200
const EXPORT_SCALE := 100

var _last_hash: int = 0
var _next_check_time: int = 0

func _enter_tree() -> void:
	_ensure_export_setting()
	set_process(true)

func _exit_tree() -> void:
	set_process(false)

# Godot editor callback for polling scene changes.
func _process(_delta: float) -> void:
	var now := Time.get_ticks_msec()
	if now < _next_check_time:
		return
	_next_check_time = now + EXPORT_CHECK_INTERVAL_MSEC
	var root := get_editor_interface().get_edited_scene_root()
	if root == null:
		return
	var current_hash := _hash_node(root)
	if current_hash == _last_hash:
		return
	_last_hash = current_hash
	_export_scene(root)

func _ensure_export_setting() -> void:
	var editor_settings := get_editor_interface().get_editor_settings()
	if editor_settings.has_setting(EXPORT_PATH_SETTING):
		return
	editor_settings.set_setting(EXPORT_PATH_SETTING, DEFAULT_EXPORT_PATH)
	editor_settings.set_initial_value(EXPORT_PATH_SETTING, DEFAULT_EXPORT_PATH, true)
	editor_settings.set_property_info(EXPORT_PATH_SETTING, {
		"name": EXPORT_PATH_SETTING,
		"type": TYPE_STRING,
		"hint": PROPERTY_HINT_FILE,
		"hint_string": "*.json"
	})

func _export_scene(root: Node) -> void:
	var payload := {
		"camera_config": _build_camera(root),
		"scene": _build_scene(root)
	}
	print("Exporting scene")
	var json_text := JSON.stringify(payload, "  ")
	var export_path := _get_export_path()
	var file := FileAccess.open(export_path, FileAccess.WRITE)
	if file == null:
		push_error("RayTracer exporter: unable to write " + export_path)
		return
	file.store_string(json_text)

func _get_export_path() -> String:
	var editor_settings := get_editor_interface().get_editor_settings()
	var raw_path := DEFAULT_EXPORT_PATH
	if editor_settings.has_setting(EXPORT_PATH_SETTING):
		raw_path = str(editor_settings.get_setting(EXPORT_PATH_SETTING))
	if raw_path.begins_with("res://") or raw_path.begins_with("user://"):
		return ProjectSettings.globalize_path(raw_path)
	return raw_path

func _build_camera(root: Node) -> Dictionary:
	var camera := _find_first_camera(root)
	if camera == null:
		return _default_camera_config()
	var aspect_ratio_w := _get_meta_float(camera, "aspect_ratio_w", 16.0)
	var aspect_ratio_h := _get_meta_float(camera, "aspect_ratio_h", 9.0)
	var aperture := _get_meta_float(camera, "aperture", 0.0)
	var dist_to_focus := _get_meta_float(camera, "dist_to_focus", EXPORT_SCALE)
	var projection_type := _get_meta_float(camera, "projection", 0.0)
	var look_from := camera.global_transform.origin
	var look_dir := camera.global_transform.basis.z
	return {
		"field_of_view": camera.fov,
		"aspect_ratio_h": aspect_ratio_h,
		"aspect_ratio_w": aspect_ratio_w,
		"aperture": aperture,
		"dist_to_focus": dist_to_focus*EXPORT_SCALE,
		"type": projection_type,
		"look_from": _vec3_dict(look_from),
		"look_dir": _vec3_dict(look_dir)
	}

func _default_camera_config() -> Dictionary:
	return {
		"field_of_view": 90.0,
		"aspect_ratio_h": 9.0,
		"aspect_ratio_w": 16.0,
		"aperture": 0.0,
		"dist_to_focus": 100.0,
		"type": 0.0,
		"look_from": _vec3_dict(Vector3.ZERO),
		"look_dir": _vec3_dict(Vector3(0.0, 0.0, -1.0))
	}

func _build_scene(root: Node) -> Dictionary:
	var objects: Array = []
	_collect_objects(root, objects, 0)
	return {
		"id": 0,
		"material_id": "",
		"type": TYPE_SCENE,
		"objects": objects
	}

func _collect_objects(node: Node, objects: Array, next_id: int) -> int:
	var current_id := next_id
	var obj: Variant = _build_object(node, current_id)
	if obj != null:
		objects.append(obj)
		current_id += 1
	for child in node.get_children():
		current_id = _collect_objects(child, objects, current_id)
	return current_id

func _build_object(node: Node, object_id: int):
	if node is MeshInstance3D:
		var mesh_instance := node as MeshInstance3D
		if mesh_instance.mesh.resource_path == "res://meshes/sphere.obj":
			return _build_sphere(mesh_instance, object_id)
		else:
			return _build_static_mesh(mesh_instance, object_id)
	return null

func _build_sphere(node: MeshInstance3D, object_id: int):
	var scale := node.global_transform.basis.get_scale()
	var radius := node.get_aabb().size.x/2 * _max_component(scale)
	return {
		"id": object_id,
		"type": TYPE_SPHERE,
		"material_id": _get_material_id(node),
		"origin": _vec3_dict(node.global_transform.origin),
		"radius": radius*EXPORT_SCALE
	}

func _build_static_mesh(node: MeshInstance3D, object_id: int):
	var euler = node.global_transform.basis.orthonormalized().get_euler()
	return {
		"id": object_id,
		"type": TYPE_STATIC_MESH,
		"material_id": _get_material_id(node),
		"file_name": node.mesh.resource_path,
		"shape_index": 0,
		"origin": _vec3_dict(node.global_transform.origin),
		"rotation": {
			"x": euler.x,
			"y": euler.y,
			"z": euler.z
		},
		"scale": _vec3_dict(node.scale)
	}

func _find_first_camera(node: Node) -> Camera3D:
	if node is Camera3D:
		return node
	for child in node.get_children():
		var cam := _find_first_camera(child)
		if cam != null:
			return cam
	return null

func _get_material_id(node: MeshInstance3D) -> String:
	return node.material_override.resource_path

func _vec3_dict(value: Vector3) -> Dictionary:
	return {
		"x": value.x*EXPORT_SCALE,
		"y": value.y*EXPORT_SCALE,
		"z": value.z*EXPORT_SCALE
	}

func _hash_node(node: Node) -> int:
	var result := _hash_combine(0, hash(node.get_class()))
	result = _hash_combine(result, hash(node.name))
	if node is Node3D:
		var node3d := node as Node3D
		result = _hash_combine(result, _hash_vector3(node3d.global_transform.origin))
		result = _hash_combine(result, _hash_vector3(node3d.global_transform.basis.orthonormalized().get_euler()))
		result = _hash_combine(result, _hash_vector3(node3d.scale))
	if node is MeshInstance3D:
		var mesh_instance := node as MeshInstance3D
		if mesh_instance.mesh != null:
			result = _hash_combine(result, hash(mesh_instance.mesh.get_class()))
			result = _hash_combine(result, hash(mesh_instance.mesh.resource_path))
			result = _hash_combine(result, hash(mesh_instance.material_override.get_class()))
			result = _hash_combine(result, hash(mesh_instance.material_override.resource_path))
			if mesh_instance.mesh is SphereMesh:
				result = _hash_combine(result, hash((mesh_instance.mesh as SphereMesh).radius))
	if node is Camera3D:
		result = _hash_combine(result, hash(node.fov))
		var meta_keys := node.get_meta_list()
		meta_keys.sort()
		for meta_key in meta_keys:
			result = _hash_combine(result, hash(meta_key))
			result = _hash_combine(result, hash(str(node.get_meta(meta_key))))
	for child in node.get_children():
		result = _hash_combine(result, _hash_node(child))
	return result

func _hash_vector3(value: Vector3) -> int:
	return _hash_combine(hash(value.x), _hash_combine(hash(value.y), hash(value.z)))

func _hash_combine(a: int, b: int) -> int:
	return hash([a, b])

func _max_component(value: Vector3) -> float:
	return max(value.x, max(value.y, value.z))
	
func _get_meta_float(node: Node, key: String, default_value: float) -> float:
	if node.has_meta(key):
		return float(node.get_meta(key))
	node.set_meta(key, default_value)
	return default_value
