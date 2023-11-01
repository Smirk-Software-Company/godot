#include "core/extension/godot_runtime_api.h"

#import "os_ios.h"

#include "main/main.h"
#include "core/os/main_loop.h"
#include "servers/display_server.h"
#include "core/extension/gdextension_manager.h"

#include "core/config/project_settings.h"

static OS_IOS *os = nullptr;

void godot_ios_plugins_initialize() {

}

void godot_ios_plugins_deinitialize() {
    
}

void godot_register_extension_library(const char* p_library_name, GDExtensionInitializationFunction p_init_function){
    String library_name = p_library_name;
    GDExtensionManager::register_direct_extension(library_name, p_init_function);
}

int godot_load_engine(int argc, char** argv) {
    os = new OS_IOS();
    
    
    // We must override main when testing is enabled
    TEST_MAIN_OVERRIDE

    Error err = Main::setup(argv[0], argc - 1, &argv[1]);
    if (err != OK) {
        return GODOT_ERROR;
    }

    return GODOT_OK;
}

int godot_start_engine(uint64_t native_window_handle) {
    if (Main::start(native_window_handle)) {
        return GODOT_OK;
    } else {
        return GODOT_ERROR;
    }
}

int godot_iterate_engine() {    // Repeat while !force_quit.
    DisplayServer::get_singleton()->process_events(); // get rid of pending events
    return Main::iteration() ? GODOT_EXIT : GODOT_OK;
}

int godot_shutdown_engine() {
    OS::get_singleton()->get_main_loop()->finalize();
    Main::cleanup();
    delete os;
    return GODOT_EXIT;
}

GodotRuntimeAPI* godot_load_library() {
    static GodotRuntimeAPI api = {
        .godot_register_extension_library = godot_register_extension_library,
        .godot_load_engine = godot_load_engine,
        .godot_start_engine = godot_start_engine,
        .godot_iterate_engine = godot_iterate_engine,
        .godot_shutdown_engine = godot_shutdown_engine
    };

    return &api;
}