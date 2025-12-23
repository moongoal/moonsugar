/*
 * Plugin interface.
 *
 * A moonsugar plugin is a dynamically linked library exposing a specific API.
 *
 * Each plugin must expose a `ms_plugin_get_def` function that returns a
 * `ms_plugin_def` pointer to its static plugin definition.
 *
 * A plugin can expose hooks automatically run on load and unload. Then,
 * each plugin is allowed to expose any functions that can be queried via
 * the plugin API.
 *
 * The plugin definition provides a free-form name, a plugin version, and the
 * moonsugar plugin interface version. This last value is used by the loading
 * mechanism to validate that the plugin is fully compatible with the running
 * executable and will fail if the interface version of the plugin mismatches
 * the one of the loader.
 */
#ifndef MS_PLUGIN_H
#define MS_PLUGIN_H

#include <moonsugar/api.h>

#define MS_PLUGIN_NAME_MAX_LEN (256u)
#define MS_PLUGIN_IFACE_VERSION (1u)

// Make a uint64 version value out of 3 uint16 values
#define MS_PLUGIN_MAKE_VERSION(major, minor, patch) (\
  ((uint64_t)(uint16_t)(patch) << 32) \
  | ((uint64_t)(uint16_t)(minor) << 16) \
  | ((uint64_t)(uint16_t)(major)))

#define MS_PLUGIN_GET_VERSION_MAJOR(version) ((version) & 0xffffU)
#define MS_PLUGIN_GET_VERSION_MINOR(version) (((version) >> 16) & 0xffffU)
#define MS_PLUGIN_GET_VERSION_PATCH(version) (((version) >> 32) & 0xffffU)

#define MS_PLUGIN_ON_LOAD_FUNCTION ms_plugin_on_load
#define MS_PLUGIN_ON_UNLOAD_FUNCTION ms_plugin_on_unload
#define MS_PLUGIN_GET_DEF_FUNCTION ms_plugin_get_def

#define MS_PLUGIN_ON_LOAD_FUNCTION_NAME MS_QUOTE(MS_PLUGIN_ON_LOAD_FUNCTION)
#define MS_PLUGIN_ON_UNLOAD_FUNCTION_NAME MS_QUOTE(MS_PLUGIN_ON_UNLOAD_FUNCTION)
#define MS_PLUGIN_GET_DEF_FUNCTION_NAME MS_QUOTE(MS_PLUGIN_GET_DEF_FUNCTION)

typedef struct {
  char name[MS_PLUGIN_NAME_MAX_LEN]; // Human-friendly plugin name
  uint64_t version; // Plugin version value (see MS_PLUGIN_MAKE_VERSION)
  uint32_t plugin_iface_version; // Moonsugar plugin interface version - must be MS_PLUGIN_IFACE_VERSION
} ms_plugin_def;

typedef void* ms_plugin;

typedef ms_plugin_def const *(*ms_plugin_get_def_clbk)(void); // Required, must return the plugin definition structure
typedef bool (*ms_plugin_on_load_clbk)(void * const ctx); // Optional, plugin load hook - return false on failure
typedef void (*ms_plugin_on_unload_clbk)(void); // Optional, plugin unload hook

ms_result MSAPI ms_plugin_load(
  char const * const path, // Path to plugin file
  void * const ctx, // Context value to pass to plugin load hook
  ms_plugin * const out_plugin, // Plugin handle result
  ms_plugin_def const ** const out_plugin_def // Plugin definition result or NULL
);

void MSAPI ms_plugin_unload(ms_plugin const plugin);

MSUSERET void* MSAPI ms_plugin_get_function(ms_plugin const plugin, char const * const name); // Get a plugin API function - returns NULL on failure

#endif // MS_PLUGIN_H
