#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <dlfcn.h>
#include <pthread.h>
#include <funchook-helper.h>

typedef struct {
  char const* const name;
  void* dl_handle;
} library_t;

#if defined(HOOK_GTK2)
#include "gtk2.h"
static library_t library_gtk2 = {
  .name = "libgtk-x11-2.0.so.0",
};
#endif

#if defined(HOOK_GTK3)
#include "gtk3.h"
static library_t library_gtk3 = {
  .name = "libgtk-3.so.0",
};
#endif

#if defined(HOOK_GTK4)
#include "gtk4.h"
static library_t library_gtk4 = {
  .name = "libgtk-4.so.1",
};
#endif

static pthread_mutex_t rtld_mutex = {};
static pthread_mutexattr_t rtld_mutex_attr = {};

static fhh_hook_state_t dlopen_hook_state = {};
static fhh_hook_state_t dlclose_hook_state = {};

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
  assert(pthread_mutex_lock(&rtld_mutex) == 0);

#if defined(HOOK_GTK2)
  bool was_gtk2_loaded = is_library_loaded(&library_gtk2);
#endif
#if defined(HOOK_GTK3)
  bool was_gtk3_loaded = is_library_loaded(&library_gtk3);
#endif
#if defined(HOOK_GTK4)
  bool was_gtk4_loaded = is_library_loaded(&library_gtk4);
#endif
  auto ret = original_dlopen(file, mode);

  if (ret == nullptr) {
    goto ret;
  }

  // Check if GTK was loaded by our dlopen() call

#if defined(HOOK_GTK2)
  bool gtk2_loaded = is_library_loaded(&library_gtk2) && !was_gtk2_loaded;
#endif
#if defined(HOOK_GTK3)
  bool gtk3_loaded = is_library_loaded(&library_gtk3) && !was_gtk3_loaded;
#endif
#if defined(HOOK_GTK4)
  bool gtk4_loaded = is_library_loaded(&library_gtk4) && !was_gtk4_loaded;
#endif

#if defined(HOOK_GTK2)
  if (gtk2_loaded) {
    if (library_gtk2.dl_handle == nullptr) {
      library_gtk2.dl_handle = original_dlopen(file, RTLD_LAZY | RTLD_NOLOAD);
      assert(library_gtk2.dl_handle != nullptr);
      hook_gtk2_install_hooks(library_gtk2.dl_handle);
    }
  }
#endif
#if defined(HOOK_GTK3)
  if (gtk3_loaded) {
    if (library_gtk3.dl_handle == nullptr) {
      library_gtk3.dl_handle = original_dlopen(file, RTLD_LAZY | RTLD_NOLOAD);
      assert(library_gtk3.dl_handle != nullptr);
      hook_gtk3_install_hooks(library_gtk3.dl_handle);
    }
  }
#endif
#if defined(HOOK_GTK4)
  if (gtk4_loaded) {
    if (library_gtk4.dl_handle == nullptr) {
      library_gtk4.dl_handle = original_dlopen(file, RTLD_LAZY | RTLD_NOLOAD);
      assert(library_gtk4.dl_handle != nullptr);
      hook_gtk4_install_hooks(library_gtk4.dl_handle);
    }
  }
#endif

ret:
  assert(pthread_mutex_unlock(&rtld_mutex) == 0);
  return ret;
}

static int dlclose_hook(void* handle) {
  FHH_ASSERT_HOOK_SIG_MATCHES(dlclose);
  assert(pthread_mutex_lock(&rtld_mutex) == 0);

#if defined(HOOK_GTK2)
  bool was_gtk2_loaded = library_gtk2.dl_handle != nullptr && handle == library_gtk2.dl_handle;
#endif
#if defined(HOOK_GTK3)
  bool was_gtk3_loaded = library_gtk3.dl_handle != nullptr && handle == library_gtk3.dl_handle;
#endif
#if defined(HOOK_GTK4)
  bool was_gtk4_loaded = library_gtk4.dl_handle != nullptr && handle == library_gtk4.dl_handle;
#endif

  auto ret = original_dlclose(handle);

  // Check if GTK was unloaded by our dlclose() call.

#if defined(HOOK_GTK2)
  if (was_gtk2_loaded) {
    assert(original_dlclose(library_gtk2.dl_handle) == 0);
    library_gtk2.dl_handle = original_dlopen(library_gtk2.name, RTLD_LAZY | RTLD_NOLOAD);
    if (library_gtk2.dl_handle == nullptr) {
      hook_gtk2_uninstall_hooks();
    }
  }
#endif
#if defined(HOOK_GTK3)
  if (was_gtk3_loaded) {
    assert(original_dlclose(library_gtk3.dl_handle) == 0);
    library_gtk3.dl_handle = original_dlopen(library_gtk3.name, RTLD_LAZY | RTLD_NOLOAD);
    if (library_gtk3.dl_handle == nullptr) {
      hook_gtk3_uninstall_hooks();
    }
  }
#endif
#if defined(HOOK_GTK4)
  if (was_gtk4_loaded) {
    assert(original_dlclose(library_gtk4.dl_handle) == 0);
    library_gtk4.dl_handle = original_dlopen(library_gtk4.name, RTLD_LAZY | RTLD_NOLOAD);
    if (library_gtk4.dl_handle == nullptr) {
      hook_gtk4_uninstall_hooks();
    }
  }
#endif

  assert(pthread_mutex_unlock(&rtld_mutex) == 0);
  return ret;
}

__attribute__((constructor))
static void init() {
  pthread_mutexattr_init(&rtld_mutex_attr);
  pthread_mutexattr_settype(&rtld_mutex_attr, PTHREAD_MUTEX_RECURSIVE_NP);
  pthread_mutex_init(&rtld_mutex, &rtld_mutex_attr);

#if defined(HOOK_GTK2)
  if (is_library_loaded(&library_gtk2)) {
    hook_gtk2_install_hooks(RTLD_DEFAULT);
    return;
  }
#endif
#if defined(HOOK_GTK3)
  if (is_library_loaded(&library_gtk3)) {
    hook_gtk3_install_hooks(RTLD_DEFAULT);
    return;
  }
#endif
#if defined(HOOK_GTK4)
  if (is_library_loaded(&library_gtk4)) {
    hook_gtk4_install_hooks(RTLD_DEFAULT);
    return;
  }
#endif

  bool dlopen_success = FHH_INSTALL(RTLD_DEFAULT, dlopen);
  bool dlclose_success = FHH_INSTALL(RTLD_DEFAULT, dlclose);
  assert(dlopen_success == dlclose_success);
}
