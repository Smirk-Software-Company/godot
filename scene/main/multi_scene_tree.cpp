#include "multi_scene_tree.h"
#include "core/config/project_settings.h"
#include "window.h"

SceneTree* MultiSceneTree::create_sub_tree(Ref<PackedScene> p_scene, uint64_t p_native_window_handle) {
	SceneTree* sub_tree = memnew(SceneTree);
	if (p_scene.is_valid()) {
		sub_tree->change_scene_to_packed(p_scene);
	}
	Window* window = memnew(Window);
	window->set_name("root");
	window->set_title(GLOBAL_GET("application/config/name"));
	window->set_use_own_world_3d(true);
	if (p_native_window_handle != 0u) {
		window->init_from_native(p_native_window_handle);
	}
	sub_tree->init_with_root(window);
	if (_initialized) {
		sub_tree->initialize();
	}
	_sub_trees.insert(sub_tree);
	return sub_tree;
}

void MultiSceneTree::destroy_sub_tree(SceneTree* p_sub_tree) {
	ERR_FAIL_NULL(p_sub_tree);
	ERR_FAIL_COND(!has_sub_tree(p_sub_tree));
	_sub_trees.erase(p_sub_tree);
	memdelete(p_sub_tree);
}

bool MultiSceneTree::has_sub_tree(SceneTree* p_sub_tree) {
	return _sub_trees.has(p_sub_tree);
}

void MultiSceneTree::initialize() {
	SceneTree::initialize();
	for (SceneTree* sub_tree : _sub_trees) {
		sub_tree->initialize();
	}
	_initialized = true;
}

bool MultiSceneTree::physics_process(double p_time) {
	_quit = SceneTree::physics_process(p_time);
	for (SceneTree* sub_tree : _sub_trees) {
		_quit |= sub_tree->physics_process(p_time);
	}
	return _quit;
}

bool MultiSceneTree::process(double p_time) {
	_quit = SceneTree::process(p_time);
	for (SceneTree* sub_tree : _sub_trees) {
		_quit |= sub_tree->process(p_time);
	}
	return _quit;
}

void MultiSceneTree::finalize() {
	SceneTree::finalize();
	for (SceneTree* sub_tree : _sub_trees) {
		sub_tree->finalize();
	}
	_initialized = false;
}

void MultiSceneTree::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_sub_tree", "scene", "native_window_handle"), &MultiSceneTree::create_sub_tree);
	ClassDB::bind_method(D_METHOD("destroy_sub_tree", "sub_tree"), &MultiSceneTree::destroy_sub_tree);
	ClassDB::bind_method(D_METHOD("has_sub_tree", "sub_tree"), &MultiSceneTree::has_sub_tree);
}