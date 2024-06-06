#include "kinematic_body_3d.h"

void KinematicBody3D::set_linear_velocity(const Vector3 &p_velocity) {
	linear_velocity = p_velocity;
}

Vector3 KinematicBody3D::get_linear_velocity() const {
	return linear_velocity;
}

void KinematicBody3D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_PHYSICS_PROCESS: {
			Transform3D transform = get_global_transform();
			set_global_transform(Transform3D(transform.basis, transform.origin + linear_velocity * get_physics_process_delta_time()));
			_on_transform_changed();
		} break;

		case NOTIFICATION_CHILD_ORDER_CHANGED: {
			for (int i = 0; i < get_child_count(false); i++) {
				Variant custom_class_name = get_child(i, false)->get("custom_class_name");

				if (custom_class_name && custom_class_name.get_type() == Variant::STRING && custom_class_name == "LuaScript") {
					PhysicsServer3D::get_singleton()->body_set_mode(get_rid(), PhysicsServer3D::BODY_MODE_KINEMATIC);
					break;
				}
			}
		} break;
	}
}

void KinematicBody3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_linear_velocity", "linear_velocity"), &KinematicBody3D::set_linear_velocity);
	ClassDB::bind_method(D_METHOD("get_linear_velocity"), &KinematicBody3D::get_linear_velocity);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "linear_velocity", PROPERTY_HINT_NONE, "suffix:m/s"), "set_linear_velocity", "get_linear_velocity");
}

KinematicBody3D::KinematicBody3D() :
		StaticBody3D(PhysicsServer3D::BODY_MODE_STATIC) {
	set_physics_process(true);
}
