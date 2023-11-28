#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <dlfcn.h>
#include <pthread.h>
#include <funchook-helper.h>

#if defined(HOOK_GTK2)
#include "gtk2.h"
#endif

#if defined(HOOK_GTK3)
#include "gtk3.h"
#endif

#if defined(HOOK_GTK4)
#include "gtk4.h"
#endif

typedef struct {
  char const* const name;
  void* dl_handle;
  bool disabled;
} library_t;

static library_t library_gtk2 = {
  .name = "libgtk-x11-2.0.so.0",
};

static library_t library_gtk3 = {
  .name = "libgtk-3.so.0",
};

static library_t library_gtk4 = {
  .name = "libgtk-4.so.1",
};

static bool hook_dlfcn_disabled = false;

static pthread_mutex_t dlfcn_mutex = {};
static pthread_mutexattr_t dlfcn_mutex_attr = {};

static fhh_hook_state_t dlopen_hook_state = {};
static fhh_hook_state_t dlclose_hook_state = {};

static void load_settings() {
  char* env;

  library_gtk2.disabled = true;
  library_gtk3.disabled = true;
  library_gtk4.disabled = true;
  hook_dlfcn_disabled = false;

  env = getenv("GTKCLIPBLOCK_HOOK");
  if (env != nullptr) {
    if (strcmp(env, "") == 0 || strcmp(env, "0") == 0) {
      // this is the default
    } else if (strcmp(env, "1") == 0) {
      library_gtk2.disabled = false;
      library_gtk3.disabled = false;
      library_gtk4.disabled = false;
    } else {
      library_gtk2.disabled = true;
      library_gtk3.disabled = true;
      library_gtk4.disabled = true;

      env = strdup(env);
      static char const* const delim = ",";
      char* tok_rest = nullptr;
      char* tok = strtok_r(env, delim, &tok_rest);
      while (tok != nullptr) {
        if (strcmp(tok, "gtk2") == 0) {
          library_gtk2.disabled = false;
        } else if (strcmp(tok, "gtk3") == 0) {
          library_gtk3.disabled = false;
        } else if (strcmp(tok, "gtk4") == 0) {
          library_gtk4.disabled = false;
        }

        tok = strtok_r(nullptr, delim, &tok_rest);
      }
      free(env);
    }
    env = nullptr;
  }

  if (library_gtk2.disabled && library_gtk3.disabled && library_gtk4.disabled) {
    hook_dlfcn_disabled = true;
  }

  env = getenv("GTKCLIPBLOCK_HOOK_DLFCN");
  if (env != nullptr) {
    if (strcmp(env, "0") == 0) {
      hook_dlfcn_disabled = true;
    }
    env = nullptr;
  }
}

static void* original_dlopen(char const* file, int mode) {
  auto dlopen_func = FHH_GET_ORIGINAL_FUNC(dlopen);

  if (dlopen_func == nullptr) {
    dlopen_func = dlopen;
  }

  return dlopen_func(file, mode);
}

static int original_dlclose(void* handle) {
  auto dlclose_func = FHH_GET_ORIGINAL_FUNC(dlclose);

  if (dlclose_func == nullptr) {
    dlclose_func = dlclose;
  }

  return dlclose_func(handle);
}

static bool is_library_loaded(library_t const* library) {
  // We rely on RTLD_NOLOAD to tell us if the shared object is mapped to memory.
  // This seems to work for:
  //   a) binaries linked at compile-time
  //   b) binaries making use of the runtime linker (dlopen/dlclose)
  auto handle = original_dlopen(library->name, RTLD_LAZY | RTLD_NOLOAD);

  if (handle != nullptr) {
    original_dlclose(handle);
    return true;
  }

  return false;
}

static void* dlopen_hook(char const* file, int mode) {
  FHH_ASSERT_HOOK_SIG_MATCHES(dlopen);
  assert(pthread_mutex_lock(&dlfcn_mutex) == 0);

  bool was_gtk2_loaded = is_library_loaded(&library_gtk2);
  bool was_gtk3_loaded = is_library_loaded(&library_gtk3);
  bool was_gtk4_loaded = is_library_loaded(&library_gtk4);
  auto ret = original_dlopen(file, mode);

  if (ret == nullptr) {
    goto ret;
  }

  // Check if GTK was loaded by our dlopen() call

  bool gtk2_loaded = !was_gtk2_loaded && is_library_loaded(&library_gtk2);
  bool gtk3_loaded = !was_gtk3_loaded && is_library_loaded(&library_gtk3);
  bool gtk4_loaded = !was_gtk4_loaded && is_library_loaded(&library_gtk4);

  if (!library_gtk2.disabled && gtk2_loaded) {
    if (library_gtk2.dl_handle == nullptr) {
#if defined(HOOK_GTK2)
      library_gtk2.dl_handle = original_dlopen(file, RTLD_LAZY | RTLD_NOLOAD);
      assert(library_gtk2.dl_handle != nullptr);
      hook_gtk2_install_hooks(library_gtk2.dl_handle);
#endif
    }
  }

  if (!library_gtk3.disabled && gtk3_loaded) {
    if (library_gtk3.dl_handle == nullptr) {
#if defined(HOOK_GTK3)
      library_gtk3.dl_handle = original_dlopen(file, RTLD_LAZY | RTLD_NOLOAD);
      assert(library_gtk3.dl_handle != nullptr);
      hook_gtk3_install_hooks(library_gtk3.dl_handle);
#endif
    }
  }

  if (!library_gtk4.disabled && gtk4_loaded) {
    if (library_gtk4.dl_handle == nullptr) {
#if defined(HOOK_GTK4)
      library_gtk4.dl_handle = original_dlopen(file, RTLD_LAZY | RTLD_NOLOAD);
      assert(library_gtk4.dl_handle != nullptr);
      hook_gtk4_install_hooks(library_gtk4.dl_handle);
#endif
    }
  }

ret:
  assert(pthread_mutex_unlock(&dlfcn_mutex) == 0);
  return ret;
}

static int dlclose_hook(void* handle) {
  FHH_ASSERT_HOOK_SIG_MATCHES(dlclose);
  assert(pthread_mutex_lock(&dlfcn_mutex) == 0);

  bool was_gtk2_loaded = library_gtk2.dl_handle != nullptr && handle == library_gtk2.dl_handle;
  bool was_gtk3_loaded = library_gtk3.dl_handle != nullptr && handle == library_gtk3.dl_handle;
  bool was_gtk4_loaded = library_gtk4.dl_handle != nullptr && handle == library_gtk4.dl_handle;

  auto ret = original_dlclose(handle);

  // Check if GTK was unloaded by our dlclose() call.

  if (was_gtk2_loaded) {
    assert(original_dlclose(library_gtk2.dl_handle) == 0);
    library_gtk2.dl_handle = original_dlopen(library_gtk2.name, RTLD_LAZY | RTLD_NOLOAD);
    if (library_gtk2.dl_handle == nullptr) {
#if defined(HOOK_GTK2)
      hook_gtk2_uninstall_hooks();
#endif
    }
  }

  if (was_gtk3_loaded) {
    assert(original_dlclose(library_gtk3.dl_handle) == 0);
    library_gtk3.dl_handle = original_dlopen(library_gtk3.name, RTLD_LAZY | RTLD_NOLOAD);
    if (library_gtk3.dl_handle == nullptr) {
#if defined(HOOK_GTK3)
      hook_gtk3_uninstall_hooks();
#endif
    }
  }

  if (was_gtk4_loaded) {
    assert(original_dlclose(library_gtk4.dl_handle) == 0);
    library_gtk4.dl_handle = original_dlopen(library_gtk4.name, RTLD_LAZY | RTLD_NOLOAD);
    if (library_gtk4.dl_handle == nullptr) {
#if defined(HOOK_GTK4)
      hook_gtk4_uninstall_hooks();
#endif
    }
  }

  assert(pthread_mutex_unlock(&dlfcn_mutex) == 0);
  return ret;
}

__attribute__((constructor))
static void init() {
  load_settings();

  if (!library_gtk2.disabled && is_library_loaded(&library_gtk2)) {
#if defined(HOOK_GTK2)
    hook_gtk2_install_hooks(RTLD_DEFAULT);
    library_gtk2.disabled = true;
#endif
  }

  if (!library_gtk3.disabled && is_library_loaded(&library_gtk3)) {
#if defined(HOOK_GTK3)
    hook_gtk3_install_hooks(RTLD_DEFAULT);
    library_gtk3.disabled = true;
#endif
  }

  if (!library_gtk4.disabled && is_library_loaded(&library_gtk4)) {
#if defined(HOOK_GTK4)
    hook_gtk4_install_hooks(RTLD_DEFAULT);
    library_gtk4.disabled = true;
#endif
  }

  if (!hook_dlfcn_disabled) {
    pthread_mutexattr_init(&dlfcn_mutex_attr);
    pthread_mutexattr_settype(&dlfcn_mutex_attr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&dlfcn_mutex, &dlfcn_mutex_attr);

    bool dlopen_success = FHH_INSTALL(RTLD_DEFAULT, dlopen);
    bool dlclose_success = FHH_INSTALL(RTLD_DEFAULT, dlclose);
    assert(dlopen_success == dlclose_success);
  }
}
