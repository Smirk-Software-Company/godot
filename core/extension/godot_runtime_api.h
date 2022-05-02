#ifdef __cplusplus

#define GODOT_OK 0
#define GODOT_ERROR -1
#define GODOT_EXIT 1

extern "C" {
#endif

#ifdef GODOT_EXTERNAL
#include <gdextension_interface.h>
#else
#include "core/extension/gdextension_interface.h"
#endif

typedef void(*godot_register_extension_library_func)(const char*, GDExtensionInitializationFunction);
typedef int(*godot_load_engine_func)(int, char**);
typedef int(*godot_start_engine_func)();
typedef int(*godot_iterate_engine_func)();
typedef int(*godot_shutdown_engine_func)();

typedef struct {
    godot_register_extension_library_func godot_register_extension_library;
    godot_load_engine_func godot_load_engine;
    godot_start_engine_func godot_start_engine;
    godot_iterate_engine_func godot_iterate_engine;
    godot_shutdown_engine_func godot_shutdown_engine;
} GodotRuntimeAPI;

typedef GodotRuntimeAPI*(*godot_load_library_func)();

GodotRuntimeAPI* godot_load_library();

#ifdef __cplusplus
}
#endif