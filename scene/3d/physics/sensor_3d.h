#include "scene/3d/area_3d.h"

class Sensor3D : public Area3D {
	GDCLASS(Sensor3D, Area3D);

private:
	Vector3 linear_velocity;

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void set_linear_velocity(const Vector3 &p_velocity);
	Vector3 get_linear_velocity() const;

	Sensor3D();

private:
};
