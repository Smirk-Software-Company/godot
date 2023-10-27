#include "external_window.h"
#include "core/error/error_macros.h"
#include "scene/scene_string_names.h"
#include "scene/theme/theme_db.h"

void ExternalWindow::set_visible(bool p_visible) {
	WARN_PRINT("Cannot set visibility of ExternalWindow");
}

void ExternalWindow::init_from_native(uint64_t p_native_window_handle) {
	native_window_handle = reinterpret_cast<void*>(p_native_window_handle);
}

void ExternalWindow::_notification(int p_what) {
	ERR_MAIN_THREAD_GUARD;
	if (p_what == NOTIFICATION_ENTER_TREE) {
		window_id = DisplayServer::get_singleton()->wrap_external_window(native_window_handle);
		notification(NOTIFICATION_VISIBILITY_CHANGED);
		emit_signal(SceneStringNames::get_singleton()->visibility_changed);
		RS::get_singleton()->viewport_set_active(get_viewport_rid(), true);
		// Emits NOTIFICATION_THEME_CHANGED internally.
		set_theme_context(ThemeDB::get_singleton()->get_nearest_theme_context(this));
	} else if (p_what == NOTIFICATION_EXIT_TREE) {
		set_theme_context(nullptr, false);
		RS::get_singleton()->viewport_set_active(get_viewport_rid(), false);
	} else {
		Window::_notification(p_what);
	}
}

void ExternalWindow::_bind_methods() {
	ClassDB::bind_method(D_METHOD("init_from_native", "native_window_handle"), &ExternalWindow::init_from_native);
}

ExternalWindow::ExternalWindow() {

}

ExternalWindow::~ExternalWindow() {

}