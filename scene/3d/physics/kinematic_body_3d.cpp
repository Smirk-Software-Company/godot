#include "kinematic_body_3d.h"

void KinematicBody3D::set_linear_velocity(const Vector3 &p_velocity) {
	linear_velocity = p_velocity;
}

Vector3 KinematicBody3D::get_linear_velocity() const {
	return linear_velocity;
}

void KinematicBody3D::set_surface_linear_velocity(const Vector3 &p_velocity) {
	surface_linear_velocity = p_velocity;
}

Vector3 KinematicBody3D::get_surface_linear_velocity() const {
	return surface_linear_velocity;
}

void KinematicBody3D::set_surface_angular_velocity(const Vector3 &p_velocity) {
	surface_angular_velocity = p_velocity;
}

Vector3 KinematicBody3D::get_surface_angular_velocity() const {
	return surface_angular_velocity;
}

void KinematicBody3D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			last_transform = get_global_transform();
		} break;

		case NOTIFICATION_PHYSICS_PROCESS: {
			Transform3D transform = get_global_transform();

			PhysicsServer3D::get_singleton()->body_set_state(get_rid(), PhysicsServer3D::BODY_STATE_LINEAR_VELOCITY, ((transform.origin - last_transform.origin) / get_physics_process_delta_time()) + surface_linear_velocity);
			PhysicsServer3D::get_singleton()->body_set_state(get_rid(), PhysicsServer3D::BODY_STATE_ANGULAR_VELOCITY, ((transform.basis * last_transform.basis.inverse()).get_euler() / get_physics_process_delta_time()) + surface_angular_velocity);

			last_transform = transform;

			set_global_transform(Transform3D(transform.basis, transform.origin + linear_velocity * get_physics_process_delta_time()));
			_on_transform_changed();
		} break;
	}
}

void KinematicBody3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_linear_velocity", "linear_velocity"), &KinematicBody3D::set_linear_velocity);
	ClassDB::bind_method(D_METHOD("get_linear_velocity"), &KinematicBody3D::get_linear_velocity);
	ClassDB::bind_method(D_METHOD("set_surface_linear_velocity", "linear_velocity"), &KinematicBody3D::set_surface_linear_velocity);
	ClassDB::bind_method(D_METHOD("get_surface_linear_velocity"), &KinematicBody3D::get_surface_linear_velocity);
	ClassDB::bind_method(D_METHOD("set_surface_angular_velocity", "linear_velocity"), &KinematicBody3D::set_surface_angular_velocity);
	ClassDB::bind_method(D_METHOD("get_surface_angular_velocity"), &KinematicBody3D::get_surface_angular_velocity);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "linear_velocity", PROPERTY_HINT_NONE, "suffix:m/s"), "set_linear_velocity", "get_linear_velocity");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "surface_linear_velocity", PROPERTY_HINT_NONE, "suffix:m/s"), "set_surface_linear_velocity", "get_surface_linear_velocity");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "surface_angular_velocity", PROPERTY_HINT_NONE, "suffix:m/s"), "set_surface_angular_velocity", "get_surface_angular_velocity");
}

KinematicBody3D::KinematicBody3D() :
		StaticBody3D(PhysicsServer3D::BODY_MODE_STATIC) {
	set_physics_process(true);
}
