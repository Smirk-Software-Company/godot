#include "sensor_3d.h"

void Sensor3D::set_linear_velocity(const Vector3 &p_velocity) {
	linear_velocity = p_velocity;
}

Vector3 Sensor3D::get_linear_velocity() const {
	return linear_velocity;
}

void Sensor3D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_PHYSICS_PROCESS: {
			Transform3D transform = get_global_transform();
			transform.set_origin(transform.origin + linear_velocity * get_physics_process_delta_time());
			set_global_transform(transform);
		}
	}
}

void Sensor3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_linear_velocity", "linear_velocity"), &Sensor3D::set_linear_velocity);
	ClassDB::bind_method(D_METHOD("get_linear_velocity"), &Sensor3D::get_linear_velocity);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "linear_velocity", PROPERTY_HINT_NONE, "suffix:m/s"), "set_linear_velocity", "get_linear_velocity");
}

Sensor3D::Sensor3D() :
		Area3D() {
	set_physics_process(true);
}
