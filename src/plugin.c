#ifdef _WIN32
  #include <Windows.h>

  #define dll_type HMODULE
  #define dll_load(path) LoadLibrary(path)
  #define dll_unload(plugin) FreeLibrary((HMODULE)(plugin))
  #define dll_get_func(plugin, name) GetProcAddress((HMODULE)(plugin), (name));
#else
  #include <dlfcn.h>

  #define dll_type void*
  #define dll_load(path) dlopen((path), RTLD_LAZY | RTLD_GLOBAL)
  #define dll_unload(plugin) dlclose(plugin)
  #define dll_get_func(plugin, name) dlsym((plugin), (name))
#endif

#include <moonsugar/plugin.h>
#include <moonsugar/log.h>

static void print_last_error(char const * const msg) {
#if _WIN32
  DWORD const error = GetLastError();

  ms_errorf("%s. Windows returned error %lu.", msg, error);
#else
  char* const error = dlerror();

  if(error == NULL) {
    error = "<No error returned>";
  }

  ms_errorf("%s. %s.", msg, error);
#endif
}

ms_result ms_plugin_load(
  char const * const path,
  void * const ctx,
  ms_plugin * const out_plugin,
  ms_plugin_def const ** const out_plugin_def
) {
#define THROW(errcode) do { error = (errcode); goto on_error; } while(false)
  ms_result error = MS_RESULT_SUCCESS;
  dll_type module = NULL;

  if(path == NULL || out_plugin == NULL) {
    THROW(MS_RESULT_INVALID_ARGUMENT);
  }

  module = dll_load(path);

  if(module == NULL) {
    print_last_error("Unable to load plugin file.");
    THROW(MS_RESULT_INVALID_ARGUMENT);
  }

  ms_plugin_get_def_clbk const get_def = (ms_plugin_get_def_clbk)ms_plugin_get_function(module, MS_PLUGIN_GET_DEF_FUNCTION_NAME);
  ms_plugin_on_load_clbk const on_load = (ms_plugin_on_load_clbk)ms_plugin_get_function(module, MS_PLUGIN_ON_LOAD_FUNCTION_NAME);

  if(get_def == NULL) {
    ms_error("Module is not plugin - missing " MS_PLUGIN_GET_DEF_FUNCTION_NAME);
    THROW(MS_RESULT_INVALID_ARGUMENT);
  }

  ms_plugin_def const * const def = get_def();

  if(def == NULL) {
    ms_error("Plugin definition is NULL.");
    THROW(MS_RESULT_INVALID_ARGUMENT);
  }

  if(def->plugin_iface_version != MS_PLUGIN_IFACE_VERSION) {
    ms_errorf(
      "Plugin \"%s\" interface version %u is not supported. Required version %u. Re-compile plugin.",
      def->name,
      def->plugin_iface_version,
      MS_PLUGIN_IFACE_VERSION
    );

    THROW(MS_RESULT_UNSUPPORTED);
  }

  *out_plugin = module;
  
  if(out_plugin_def != NULL) {
    *out_plugin_def = def;
  }

  if(on_load != NULL) {
    if(!on_load(ctx)) {
      ms_errorf("Plugin \"%s\" failed startup.", def->name);
      THROW(MS_RESULT_UNKNOWN);
    }
  }

  ms_debugf(
    "Loaded plugin \"%s\", version %llu.%llu.%llu.",
    def->name,
    MS_PLUGIN_GET_VERSION_MAJOR(def->version),
    MS_PLUGIN_GET_VERSION_MINOR(def->version),
    MS_PLUGIN_GET_VERSION_PATCH(def->version)
  );

  goto on_success;
#undef THROW

on_error:
  dll_unload(module);

on_success:
  return error;
}

void ms_plugin_unload(ms_plugin const plugin) {
  ms_plugin_on_unload_clbk const on_unload = (ms_plugin_on_unload_clbk)ms_plugin_get_function(plugin, MS_PLUGIN_ON_UNLOAD_FUNCTION_NAME);

  if(on_unload != NULL) {
    on_unload();
  }

  dll_unload((dll_type)plugin);
}

void* ms_plugin_get_function(ms_plugin const plugin, char const * const name) {
  return (void*)dll_get_func(plugin, name);
}
