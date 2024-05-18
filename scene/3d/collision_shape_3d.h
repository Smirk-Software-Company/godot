/**************************************************************************/
/*  collision_shape_3d.h                                                  */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef COLLISION_SHAPE_3D_H
#define COLLISION_SHAPE_3D_H

#include "scene/3d/node_3d.h"
#include "scene/resources/shape_3d.h"

class CollisionObject3D;
class CollisionShape3D : public Node3D {
	GDCLASS(CollisionShape3D, Node3D);

	Ref<Shape3D> shape;

	uint32_t owner_id = 0;
	CollisionObject3D *collision_object = nullptr;

	RBMap<uint32_t, CollisionShape3D *> compound_shapes;
	RBMap<uint32_t, CollisionShape3D *> compound_owners;

#ifndef DISABLE_DEPRECATED
	void resource_changed(Ref<Resource> res);
#endif
	bool disabled = false;
	bool compound = false;

protected:
	void _update_in_shape_owner(uint32_t id, bool p_xform_only = false);
	void _setup_compound_shapes();
	void _dismantle_compound_shapes();
	void _register_compound_shape(CollisionShape3D *p_shape);
	void _deregister_compound_shape(uint32_t id);
	Transform3D _get_shape_relative_transform(uint32_t id);
	void _add_child_listeners();
	void _remove_child_listeners();
	void _child_added(Node *p_node);
	void _child_removed(Node *p_node);

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void make_convex_from_siblings();

	void set_shape(const Ref<Shape3D> &p_shape);
	Ref<Shape3D> get_shape() const;

	void set_disabled(bool p_disabled);
	bool is_disabled() const;

	void set_compound(bool p_compound);
	bool is_compound() const;

	Ref<ArrayMesh> get_debug_mesh();

	PackedStringArray get_configuration_warnings() const override;

	CollisionShape3D();
	~CollisionShape3D();
};

#endif // COLLISION_SHAPE_3D_H
