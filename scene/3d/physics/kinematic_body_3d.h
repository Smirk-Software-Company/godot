#include "scene/3d/physics_body_3d.h"

class KinematicBody3D : public StaticBody3D {
	GDCLASS(KinematicBody3D, StaticBody3D);

private:
	Vector3 linear_velocity;

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void set_linear_velocity(const Vector3 &p_velocity);
	Vector3 get_linear_velocity() const override;

	KinematicBody3D();

private:
};
