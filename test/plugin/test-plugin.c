#include <moonsugar/plugin.h>

#ifdef _WIN32
  #define PLUGAPI __attribute__((dllexport))
#else
  #define PLUGAPI
#endif

static ms_plugin_def const plugin_def = {
  "Test Plugin",
  MS_PLUGIN_MAKE_VERSION(1, 0, 0),
  MS_PLUGIN_IFACE_VERSION
};

static bool *state;

ms_plugin_def const * PLUGAPI MS_PLUGIN_GET_DEF_FUNCTION(void) {
  return &plugin_def;
}

bool PLUGAPI MS_PLUGIN_ON_LOAD_FUNCTION(void * const ctx) {
  if(ctx == NULL) {
    return false;
  }

  state = ctx;
  *state = true;

  return true;
}

void PLUGAPI MS_PLUGIN_ON_UNLOAD_FUNCTION(void) {
  if(state != NULL) {
    *state = false;
    state = NULL;
  }
}

int PLUGAPI plug_add(int a, int b) {
  return a + b;
}
