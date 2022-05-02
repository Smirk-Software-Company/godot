#include "core/extension/godot_runtime_api.h"

#include <limits.h>
#include <locale.h>
#include <stdlib.h>
#include <unistd.h>

#if defined(SANITIZERS_ENABLED)
#include <sys/resource.h>
#endif

#include "main/main.h"
#include "platform/linuxbsd/os_linuxbsd.h"
#include "core/os/main_loop.h"
#include "servers/display_server.h"
#include "core/extension/gdextension_manager.h"

#include "core/config/project_settings.h"

OS_LinuxBSD os;    // Only meant to be used on Linux for now.

void godot_register_extension_library(const char* p_library_name, GDExtensionInitializationFunction p_init_function){
    String library_name = p_library_name;
    GDExtensionManager::register_custom_extension(library_name, p_init_function);
}

int godot_load_engine(int argc, char** argv){

    #if defined(SANITIZERS_ENABLED)
        // Note: Set stack size to be at least 30 MB (vs 8 MB default) to avoid overflow, address sanitizer can increase stack usage up to 3 times.
        struct rlimit stack_lim = { 0x1E00000, 0x1E00000 };
        setrlimit(RLIMIT_STACK, &stack_lim);
    #endif

    setlocale(LC_CTYPE, "");

    // We must override main when testing is enabled
    TEST_MAIN_OVERRIDE

    char *cwd = (char *)malloc(PATH_MAX);
    ERR_FAIL_COND_V(!cwd, ERR_OUT_OF_MEMORY);

    Error err = Main::setup(argv[0], argc - 1, &argv[1]);
    if (err != OK) {
        free(cwd);
        return GODOT_ERROR;
    }

    return GODOT_OK;
}

int godot_start_engine(){
    if (Main::start()) {
        OS::get_singleton()->get_main_loop()->initialize();
        return GODOT_OK;
    } else {
        return GODOT_ERROR;
    }
}

int godot_iterate_engine(){    // Repeat while !force_quit.
    DisplayServer::get_singleton()->process_events(); // get rid of pending events
    return Main::iteration() ? GODOT_EXIT : GODOT_OK;
}

int godot_shutdown_engine(){
    OS::get_singleton()->get_main_loop()->finalize();
    Main::cleanup();
    return GODOT_EXIT;
}

GodotRuntimeAPI* godot_load_library(){
    static GodotRuntimeAPI api = {
        .godot_register_extension_library = godot_register_extension_library,
        .godot_load_engine = godot_load_engine,
        .godot_start_engine = godot_start_engine,
        .godot_iterate_engine = godot_iterate_engine,
        .godot_shutdown_engine = godot_shutdown_engine
    };

    return &api;
}