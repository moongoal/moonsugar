#include <string.h>
#include <moondance/test.h>
#include <moonsugar/plugin.h>

#ifdef _WIN32
  #define PLUGIN_EXT ".dll"
#else
  #define PLUGIN_EXT ".so"
#endif

#define PLUGIN_NAME "test-plugin" PLUGIN_EXT

typedef int (*plug_add_func)(int a, int b);

MD_CASE(plugin) {
  ms_plugin plugin = NULL;
  ms_plugin_def const *def = NULL;
  bool ctx = false;

  ms_result const load_result = ms_plugin_load(PLUGIN_NAME, &ctx, &plugin, &def);
  md_assert(load_result == MS_RESULT_SUCCESS);
  md_assert(ctx == true); // Set by load hook
  md_assert(plugin != NULL);
  md_assert(def != NULL);
  md_assert(strcmp("Test Plugin", def->name) == 0);
  md_assert(MS_PLUGIN_GET_VERSION_MAJOR(def->version) == 1);
  md_assert(MS_PLUGIN_GET_VERSION_MINOR(def->version) == 0);
  md_assert(MS_PLUGIN_GET_VERSION_PATCH(def->version) == 0);

  plug_add_func plug_add = (plug_add_func)ms_plugin_get_function(plugin, "plug_add");
  md_assert(plug_add != NULL);
  md_assert(plug_add(1, 2) == 3);

  plug_add_func plug_add_ne = (plug_add_func)ms_plugin_get_function(plugin, "plug_add_non_existing");
  md_assert(plug_add_ne == NULL);

  ms_plugin_unload(plugin);
  md_assert(ctx == false); // Set by unload hook
}

MD_CASE(plugin__load_hook_returns_false) {
  ms_plugin plugin = NULL;

  ms_result const load_result = ms_plugin_load(PLUGIN_NAME, NULL, &plugin, NULL);
  md_assert(load_result != MS_RESULT_SUCCESS);
  ms_plugin_unload(plugin);
}

int main(int argc, char **argv) {
  md_suite suite = md_suite_create();

  md_add(&suite, plugin);
  md_add(&suite, plugin__load_hook_returns_false);

  return md_run(argc, argv, &suite);
}
