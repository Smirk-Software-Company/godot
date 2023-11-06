#ifndef MULTI_SCENE_TREE_H
#define MULTI_SCENE_TREE_H

#include "scene/main/scene_tree.h"
#include "core/templates/hash_set.h"
#include "core/object/ref_counted.h"
#include "scene/resources/packed_scene.h"

class MultiSceneTree : public SceneTree {
	_THREAD_SAFE_CLASS_

	GDCLASS(MultiSceneTree, SceneTree);


private:
	HashSet<SceneTree*> _sub_trees;
	bool _initialized = false;

protected:
	static void _bind_methods();

public:
	SceneTree* create_sub_tree(Ref<PackedScene> p_scene = Ref<PackedScene>(), uint64_t p_native_window_handle = 0u);
	void destroy_sub_tree(SceneTree* p_sub_tree);
	bool has_sub_tree(SceneTree* p_sub_tree);

	virtual void initialize() override;
	virtual bool physics_process(double p_time) override;
	virtual bool process(double p_time) override;
	virtual void finalize() override;
};

#endif // MULTI_SCENE_TREE_H
