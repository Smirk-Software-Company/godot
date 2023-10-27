#ifndef EXTERNAL_WINDOW_H
#define EXTERNAL_WINDOW_H

#include "scene/main/window.h"

class ExternalWindow : public Window {
	GDCLASS(ExternalWindow, Window);

private:
	void* native_window_handle;

public:
	virtual void set_visible(bool p_visible) override;

	void init_from_native(uint64_t p_native_window_handle);
	void release_native();

	void _notification(int p_what);
	static void _bind_methods();

	ExternalWindow();
	~ExternalWindow();
};

#endif // EXTERNAL_WINDOW_H
