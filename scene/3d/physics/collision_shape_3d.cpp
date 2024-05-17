/**************************************************************************/
/*  collision_shape_3d.cpp                                                */
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

#include "collision_shape_3d.h"

#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/physics/character_body_3d.h"
#include "scene/3d/physics/physics_body_3d.h"
#include "scene/3d/physics/vehicle_body_3d.h"
#include "scene/resources/3d/concave_polygon_shape_3d.h"
#include "scene/resources/3d/convex_polygon_shape_3d.h"
#include "scene/resources/3d/world_boundary_shape_3d.h"

Ref<ArrayMesh> CollisionShape3D::get_debug_mesh() {
	if (shape.is_valid()) {
		return shape->get_debug_mesh();
	}

	if (compound) {
		Ref<ArrayMesh> compound_mesh = memnew(ArrayMesh);
		for (KeyValue<uint32_t, CollisionShape3D *> &E : compound_shapes) {
			if (E.value->shape.is_valid()) {
				Ref<ArrayMesh> child_mesh = E.value->shape->get_debug_mesh();
				if (child_mesh.is_valid()) {
					for (int i = 0; i < child_mesh->get_surface_count(); ++i) {
						Array surface = child_mesh->surface_get_arrays(i);
						compound_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_LINES, surface);
					}
				}
			}
		}
		return compound_mesh;
	}

	return nullptr;
}

void CollisionShape3D::make_convex_from_siblings() {
	Node *p = get_parent();
	if (!p) {
		return;
	}

	Vector<Vector3> vertices;

	for (int i = 0; i < p->get_child_count(); i++) {
		Node *n = p->get_child(i);
		MeshInstance3D *mi = Object::cast_to<MeshInstance3D>(n);
		if (mi) {
			Ref<Mesh> m = mi->get_mesh();
			if (m.is_valid()) {
				for (int j = 0; j < m->get_surface_count(); j++) {
					Array a = m->surface_get_arrays(j);
					if (!a.is_empty()) {
						Vector<Vector3> v = a[RenderingServer::ARRAY_VERTEX];
						for (int k = 0; k < v.size(); k++) {
							vertices.append(mi->get_transform().xform(v[k]));
						}
					}
				}
			}
		}
	}

	Ref<ConvexPolygonShape3D> shape_new = memnew(ConvexPolygonShape3D);
	shape_new->set_points(vertices);
	set_shape(shape_new);
}

Transform3D CollisionShape3D::_get_shape_relative_transform(uint32_t id) {
	if (id == owner_id) {
		return get_transform();
	}

	if (compound_shapes.has(id)) {
		// Returns the local transform of the shape relative to this transform
		return get_global_transform().affine_inverse() * compound_shapes[id]->get_global_transform();
	}

	return Transform3D();
}

void CollisionShape3D::_update_in_shape_owner(uint32_t id, bool p_xform_only) {
	collision_object->shape_owner_set_transform(id, _get_shape_relative_transform(id));
	if (p_xform_only) {
		return;
	}
	collision_object->shape_owner_set_disabled(id, disabled);
}

void CollisionShape3D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_PARENTED: {
			collision_object = Object::cast_to<CollisionObject3D>(get_parent());
			if (collision_object) {
				owner_id = collision_object->create_shape_owner(this);
				if (shape.is_valid()) {
					collision_object->shape_owner_add_shape(owner_id, shape);
				}
				_update_in_shape_owner(owner_id);

				if (compound) {
					_add_child_listeners();
					_setup_compound_shapes();
				}
			}
		} break;

		case NOTIFICATION_ENTER_TREE: {
			if (collision_object) {
				_update_in_shape_owner(owner_id);

				if (compound) {
					for (KeyValue<uint32_t, CollisionShape3D *> &E : compound_shapes) {
						_update_in_shape_owner(E.key);
					}
				}
			}
			break;

			case NOTIFICATION_LOCAL_TRANSFORM_CHANGED: {
				if (collision_object) {
					_update_in_shape_owner(owner_id, true);

					if (compound) {
						for (KeyValue<uint32_t, CollisionShape3D *> &E : compound_shapes) {
							_update_in_shape_owner(E.key, true);
						}
					}
				}
				update_configuration_warnings();
			} break;

			case NOTIFICATION_TRANSFORM_CHANGED: {
				for (KeyValue<uint32_t, CollisionShape3D *> &E : compound_owners) {
					if (E.value->collision_object) {
						E.value->_update_in_shape_owner(E.key, true);
					}
				}
			} break;

			case NOTIFICATION_UNPARENTED: {
				if (collision_object) {
					collision_object->remove_shape_owner(owner_id);

					if (compound) {
						_remove_child_listeners();
						_dismantle_compound_shapes();
					}
				}
				owner_id = 0;
				collision_object = nullptr;
			} break;

			case NOTIFICATION_ENABLED: {
				// TODO: tell compound owners?
			} break;

			case NOTIFICATION_DISABLED: {
				// TODO: tell compound owners?
			} break;
		}
	}
}

#ifndef DISABLE_DEPRECATED
void CollisionShape3D::resource_changed(Ref<Resource> res) {
}
#endif

PackedStringArray CollisionShape3D::get_configuration_warnings() const {
	PackedStringArray warnings = Node::get_configuration_warnings();

	CollisionObject3D *col_object = Object::cast_to<CollisionObject3D>(get_parent());
	if (col_object == nullptr) {
		warnings.push_back(RTR("CollisionShape3D only serves to provide a collision shape to a CollisionObject3D derived node.\nPlease only use it as a child of Area3D, StaticBody3D, RigidBody3D, CharacterBody3D, etc. to give them a shape."));
	}

	if (!shape.is_valid()) {
		warnings.push_back(RTR("A shape must be provided for CollisionShape3D to function. Please create a shape resource for it."));
	}

	if (shape.is_valid() && Object::cast_to<RigidBody3D>(col_object)) {
		String body_type = "RigidBody3D";
		if (Object::cast_to<VehicleBody3D>(col_object)) {
			body_type = "VehicleBody3D";
		}

		if (Object::cast_to<ConcavePolygonShape3D>(*shape)) {
			warnings.push_back(vformat(RTR("When used for collision, ConcavePolygonShape3D is intended to work with static CollisionObject3D nodes like StaticBody3D.\nIt will likely not behave well for %ss (except when frozen and freeze_mode set to FREEZE_MODE_STATIC)."), body_type));
		} else if (Object::cast_to<WorldBoundaryShape3D>(*shape)) {
			warnings.push_back(RTR("WorldBoundaryShape3D doesn't support RigidBody3D in another mode than static."));
		}
	}

	if (shape.is_valid() && Object::cast_to<CharacterBody3D>(col_object)) {
		if (Object::cast_to<ConcavePolygonShape3D>(*shape)) {
			warnings.push_back(RTR("When used for collision, ConcavePolygonShape3D is intended to work with static CollisionObject3D nodes like StaticBody3D.\nIt will likely not behave well for CharacterBody3Ds."));
		}
	}

	Vector3 scale = get_transform().get_basis().get_scale();
	if (!(Math::is_zero_approx(scale.x - scale.y) && Math::is_zero_approx(scale.y - scale.z))) {
		warnings.push_back(RTR("A non-uniformly scaled CollisionShape3D node will probably not function as expected.\nPlease make its scale uniform (i.e. the same on all axes), and change the size of its shape resource instead."));
	}

	return warnings;
}

void CollisionShape3D::_bind_methods() {
#ifndef DISABLE_DEPRECATED
	ClassDB::bind_method(D_METHOD("resource_changed", "resource"), &CollisionShape3D::resource_changed);
#endif
	ClassDB::bind_method(D_METHOD("set_shape", "shape"), &CollisionShape3D::set_shape);
	ClassDB::bind_method(D_METHOD("get_shape"), &CollisionShape3D::get_shape);
	ClassDB::bind_method(D_METHOD("set_disabled", "enable"), &CollisionShape3D::set_disabled);
	ClassDB::bind_method(D_METHOD("is_disabled"), &CollisionShape3D::is_disabled);
	ClassDB::bind_method(D_METHOD("set_compound", "compound"), &CollisionShape3D::set_compound);
	ClassDB::bind_method(D_METHOD("is_compound"), &CollisionShape3D::is_compound);
	ClassDB::bind_method(D_METHOD("make_convex_from_siblings"), &CollisionShape3D::make_convex_from_siblings);
	ClassDB::bind_method(D_METHOD("get_debug_mesh"), &CollisionShape3D::get_debug_mesh);
	ClassDB::set_method_flags("CollisionShape3D", "make_convex_from_siblings", METHOD_FLAGS_DEFAULT | METHOD_FLAG_EDITOR);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "shape", PROPERTY_HINT_RESOURCE_TYPE, "Shape3D"), "set_shape", "get_shape");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "disabled"), "set_disabled", "is_disabled");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "compound"), "set_compound", "is_compound");
}

void CollisionShape3D::set_shape(const Ref<Shape3D> &p_shape) {
	if (p_shape == shape) {
		return;
	}
	if (compound) {
		set_compound(false);
	}
	if (shape.is_valid()) {
		shape->disconnect_changed(callable_mp((Node3D *)this, &Node3D::update_gizmos));
	}
	shape = p_shape;
	if (shape.is_valid()) {
		shape->connect_changed(callable_mp((Node3D *)this, &Node3D::update_gizmos));
	}
	update_gizmos();
	if (collision_object) {
		collision_object->shape_owner_clear_shapes(owner_id);
		if (shape.is_valid()) {
			collision_object->shape_owner_add_shape(owner_id, shape);
		}
	}

	if (is_inside_tree() && collision_object) {
		// If this is a heightfield shape our center may have changed
		_update_in_shape_owner(owner_id, true);
	}
	update_configuration_warnings();
}

Ref<Shape3D> CollisionShape3D::get_shape() const {
	return shape;
}

void CollisionShape3D::set_disabled(bool p_disabled) {
	disabled = p_disabled;
	update_gizmos();
	if (collision_object) {
		collision_object->shape_owner_set_disabled(owner_id, p_disabled);
		for (KeyValue<uint32_t, CollisionShape3D *> &E : compound_shapes) {
			collision_object->shape_owner_set_disabled(E.key, p_disabled);
		}
	}
}

bool CollisionShape3D::is_disabled() const {
	return disabled;
}

void CollisionShape3D::_setup_compound_shapes() {
	TypedArray<Node> collision_shapes = find_children("*", "CollisionShape3D", true, false);
	for (int i = 0; i < collision_shapes.size(); i++) {
		// Node *node = Object::cast_to<Node>(collision_shapes[i]);

		// // // Ignore internal nodes
		// // if (node->data.internal_mode != Node::INTERNAL_MODE_DISABLED)
		// // 	continue;

		CollisionShape3D *shape = Object::cast_to<CollisionShape3D>(collision_shapes[i]);

		if (shape == nullptr) {
			continue;
		}

		_register_compound_shape(shape);
	}
}

void CollisionShape3D::_dismantle_compound_shapes() {
	for (KeyValue<uint32_t, CollisionShape3D *> &E : compound_shapes) {
		_deregister_compound_shape(E.key);
	}

	compound_shapes.clear();
}

void CollisionShape3D::_deregister_compound_shape(uint32_t id) {
	collision_object->remove_shape_owner(id);

	if (!compound_shapes.has(id)) {
		return;
	}

	printf("deregistering compound shape\n");

	CollisionShape3D *compound_shape = compound_shapes[id];
	compound_shape->set_notify_transform(false);
	compound_shape->compound_owners.erase(id);

	if (compound_shape->is_connected("tree_exiting", callable_mp(this, &CollisionShape3D::_child_removed))) {
		compound_shape->disconnect("tree_exiting", callable_mp(this, &CollisionShape3D::_child_removed));
	}

	if (compound_shape->shape.is_valid()) {
		// TODO: unregister shape changed
	}

	compound_shapes.erase(id);
}

void CollisionShape3D::_register_compound_shape(CollisionShape3D *p_shape) {
	// TODO: avoid registering duplicates -- check if it exists first
	if (!collision_object) {
		return;
	}

	printf("registering compound shape\n");

	uint32_t id = collision_object->create_shape_owner(p_shape);

	compound_shapes[id] = p_shape;

	p_shape->set_notify_transform(true);
	p_shape->compound_owners[id] = this;

	if (!p_shape->is_connected("tree_exiting", callable_mp(this, &CollisionShape3D::_child_removed))) {
		p_shape->connect("tree_exiting", callable_mp(this, &CollisionShape3D::_child_removed));
	}

	if (p_shape->shape.is_valid()) {
		collision_object->shape_owner_add_shape(id, p_shape->shape);
		// TODO: register shape changed
	}

	_update_in_shape_owner(id);

	// add shape to owner & add to our local map cache
	// add listeners for this shape being added to and removed from tree
	// listener for its global transform being updated
	// should probably accept a compound parent too so it can quickly passup the global transform. only thing is this will create a circular reference but it should be fine since we have manual memory management. prob just need to kill the refs in the deinit. either that or use a weak ptr to the compound parent (might be better)
}

void CollisionShape3D::_add_child_listeners() {
	if (!collision_object->is_connected("recursive_child_entered_tree", callable_mp(this, &CollisionShape3D::_child_added))) {
		collision_object->connect("recursive_child_entered_tree", callable_mp(this, &CollisionShape3D::_child_added));
	}
}

void CollisionShape3D::_remove_child_listeners() {
	if (collision_object->is_connected("recursive_child_entered_tree", callable_mp(this, &CollisionShape3D::_child_added))) {
		collision_object->disconnect("recursive_child_entered_tree", callable_mp(this, &CollisionShape3D::_child_added));
	}
}

void CollisionShape3D::_child_added(Node *p_node) {
	if (!compound) {
		return;
	}

	printf("child added %s\n", p_node->to_string().utf8().get_data());

	CollisionShape3D *shape = Object::cast_to<CollisionShape3D>(p_node);
	if (shape) {
		_register_compound_shape(shape);
	}
}

void CollisionShape3D::_child_removed(Node *p_node) {
	if (!compound) {
		return;
	}

	printf("child removed %s\n", p_node->to_string().utf8().get_data());

	CollisionShape3D *shape = Object::cast_to<CollisionShape3D>(p_node);

	if (shape) {
		for (KeyValue<uint32_t, CollisionShape3D *> &E : compound_shapes) {
			if (E.value == shape) {
				_deregister_compound_shape(E.key);
			}
		}
	}
}

void CollisionShape3D::set_compound(bool p_compound) {
	if (compound == p_compound) {
		return;
	}

	printf("setting compound %s\n", p_compound ? "true" : "false");

	compound = p_compound;

	if (compound) {
		_setup_compound_shapes();

		if (collision_object) {
			_add_child_listeners();
		}
	} else {
		_dismantle_compound_shapes();

		if (collision_object) {
			_remove_child_listeners();
		}
	}

	// need to maintain a list of the child collision shapes (prob has to be a map of id to shape node liek the shapes property on collision object 3d so we can pass over the correct id for updating transform and such)
	// when any are added or removed, we keep the list up to date
	// when the transform of those shapes relative to this one changes, we need to pass that over to the collision object
	// look at the process mode of the children as well to determine if they are active (prob) NOTIFICATION_DISABLED & NOTIFICATION_ENABLED

	// maybe we do something like "add compound owner" for other shapes to easily pass up their transforms when they change cause if not idk how we'll wire up the global transform changes
}

bool CollisionShape3D::is_compound() const {
	return compound;
}

CollisionShape3D::CollisionShape3D() {
	//indicator = RenderingServer::get_singleton()->mesh_create();
	set_notify_local_transform(true);
}

CollisionShape3D::~CollisionShape3D() {
	//RenderingServer::get_singleton()->free(indicator);
}
