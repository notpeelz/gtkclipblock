#include <assert.h>
#include <dlfcn.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include <funchook-helper.h>
#include "gtk4.h"

typedef struct {
  GdkClipboard* clipboard;
} tls_data_t;

static pthread_key_t tls_key;
static pthread_once_t tls_key_once = PTHREAD_ONCE_INIT;

static void create_tls_key() {
  assert(pthread_key_create(&tls_key, free) == 0);
}

static tls_data_t* get_tls_data() {
  pthread_once(&tls_key_once, create_tls_key);
  auto ptr = (tls_data_t*)pthread_getspecific(tls_key);

  if (ptr == nullptr) {
    ptr = (tls_data_t*)malloc(sizeof(tls_data_t));
    assert(ptr != nullptr);
    ptr->clipboard = nullptr;

    assert(pthread_setspecific(tls_key, ptr) == 0);
  }

  return ptr;
}

static void tls_data_set_clipboard(GdkClipboard* clipboard) {
  auto tls_data = get_tls_data();
  tls_data->clipboard = clipboard;
}

static bool tls_data_clipboard_matches(GdkClipboard* clipboard) {
  auto tls_data = get_tls_data();
  if (clipboard == nullptr) {
    return false;
  }

  return tls_data->clipboard == clipboard;
}

static typeof(&gdk_clipboard_get_display) gdk_clipboard_get_display_func = nullptr;
static typeof(&gdk_display_get_primary_clipboard) gdk_display_get_primary_clipboard_func = nullptr;

static void initialize_helper_symbols(void* handle) {
  gdk_clipboard_get_display_func =
    (typeof(&gdk_clipboard_get_display))dlsym(handle, "gdk_clipboard_get_display");
  assert(gdk_clipboard_get_display_func != nullptr);
  gdk_display_get_primary_clipboard_func =
    (typeof(&gdk_display_get_primary_clipboard))dlsym(handle, "gdk_display_get_primary_clipboard");
  assert(gdk_display_get_primary_clipboard_func != nullptr);
}

static GdkDisplay* original_gdk_clipboard_get_display(GdkClipboard* clipboard) {
  assert(gdk_clipboard_get_display_func != nullptr);
  return gdk_clipboard_get_display_func(clipboard);
}

static GdkClipboard* original_gdk_display_get_primary_clipboard(GdkDisplay* display) {
  assert(gdk_display_get_primary_clipboard_func != nullptr);
  return gdk_display_get_primary_clipboard_func(display);
}

static fhh_hook_state_t gdk_clipboard_read_async_hook_state = {};
static void gdk_clipboard_read_async_hook(
  GdkClipboard* clipboard,
  char const** mime_types,
  int io_priority,
  GCancellable* cancellable,
  GAsyncReadyCallback callback,
  gpointer user_data
) {
  FHH_ASSERT_HOOK_SIG_MATCHES(gdk_clipboard_read_async);
  auto func = FHH_GET_ORIGINAL_FUNC(gdk_clipboard_read_async);

  if (clipboard != nullptr) {
    auto display = original_gdk_clipboard_get_display(clipboard);
    if (
      display != nullptr
      && original_gdk_display_get_primary_clipboard(display) == clipboard
    ) {
      tls_data_set_clipboard(clipboard);
      callback((GObject*)clipboard, nullptr, user_data);
      tls_data_set_clipboard(nullptr);
      return;
    }
  }

  func(
    clipboard,
    mime_types,
    io_priority,
    cancellable,
    callback,
    user_data
  );
}

static fhh_hook_state_t gdk_clipboard_read_finish_hook_state = {};
static GInputStream* gdk_clipboard_read_finish_hook(
  GdkClipboard* clipboard,
  GAsyncResult* result,
  char const** out_mime_type,
  GError** error
) {
  FHH_ASSERT_HOOK_SIG_MATCHES(gdk_clipboard_read_finish);
  auto func = FHH_GET_ORIGINAL_FUNC(gdk_clipboard_read_finish);

  if (tls_data_clipboard_matches(clipboard)) {
    if (out_mime_type != nullptr) {
      *out_mime_type = '\0';
    }
    if (error != nullptr) {
      *error = nullptr;
    }
    return nullptr;
  }

  return func(
    clipboard,
    result,
    out_mime_type,
    error
  );
}

static fhh_hook_state_t gdk_clipboard_read_value_async_hook_state = {};
static void gdk_clipboard_read_value_async_hook(
  GdkClipboard* clipboard,
  GType type,
  int io_priority,
  GCancellable* cancellable,
  GAsyncReadyCallback callback,
  gpointer user_data
) {
  FHH_ASSERT_HOOK_SIG_MATCHES(gdk_clipboard_read_value_async);
  auto func = FHH_GET_ORIGINAL_FUNC(gdk_clipboard_read_value_async);

  if (clipboard != nullptr) {
    auto display = original_gdk_clipboard_get_display(clipboard);
    if (
      display != nullptr
      && original_gdk_display_get_primary_clipboard(display) == clipboard
    ) {
      tls_data_set_clipboard(clipboard);
      callback((GObject*)clipboard, nullptr, user_data);
      tls_data_set_clipboard(nullptr);
      return;
    }
  }

  func(
    clipboard,
    type,
    io_priority,
    cancellable,
    callback,
    user_data
  );
}

static fhh_hook_state_t gdk_clipboard_read_value_finish_hook_state = {};
static GValue const* gdk_clipboard_read_value_finish_hook(
  GdkClipboard* clipboard,
  GAsyncResult* result,
  GError** error
) {
  FHH_ASSERT_HOOK_SIG_MATCHES(gdk_clipboard_read_value_finish);
  auto func = FHH_GET_ORIGINAL_FUNC(gdk_clipboard_read_value_finish);

  if (tls_data_clipboard_matches(clipboard)) {
    if (error != nullptr) {
      *error = nullptr;
    }
    return nullptr;
  }

  return func(
    clipboard,
    result,
    error
  );
}

static fhh_hook_state_t gdk_clipboard_read_text_async_hook_state = {};
static void gdk_clipboard_read_text_async_hook(
  GdkClipboard* clipboard,
  GCancellable* cancellable,
  GAsyncReadyCallback callback,
  gpointer user_data
) {
  FHH_ASSERT_HOOK_SIG_MATCHES(gdk_clipboard_read_text_async);
  auto func = FHH_GET_ORIGINAL_FUNC(gdk_clipboard_read_text_async);

  if (clipboard != nullptr) {
    auto display = original_gdk_clipboard_get_display(clipboard);
    if (
      display != nullptr
      && original_gdk_display_get_primary_clipboard(display) == clipboard
    ) {
      tls_data_set_clipboard(clipboard);
      callback((GObject*)clipboard, nullptr, user_data);
      tls_data_set_clipboard(nullptr);
      return;
    }
  }

  func(
    clipboard,
    cancellable,
    callback,
    user_data
  );
}

static fhh_hook_state_t gdk_clipboard_read_text_finish_hook_state = {};
static char* gdk_clipboard_read_text_finish_hook(
  GdkClipboard* clipboard,
  GAsyncResult* result,
  GError** error
) {
  FHH_ASSERT_HOOK_SIG_MATCHES(gdk_clipboard_read_text_finish);
  auto func = FHH_GET_ORIGINAL_FUNC(gdk_clipboard_read_text_finish);

  if (tls_data_clipboard_matches(clipboard)) {
    if (error != nullptr) {
      *error = nullptr;
    }
    return nullptr;
  }

  return func(
    clipboard,
    result,
    error
  );
}

static fhh_hook_state_t gdk_clipboard_read_texture_async_hook_state = {};
static void gdk_clipboard_read_texture_async_hook(
  GdkClipboard* clipboard,
  GCancellable* cancellable,
  GAsyncReadyCallback callback,
  gpointer user_data
) {
  FHH_ASSERT_HOOK_SIG_MATCHES(gdk_clipboard_read_texture_async);
  auto func = FHH_GET_ORIGINAL_FUNC(gdk_clipboard_read_texture_async);

  if (clipboard != nullptr) {
    auto display = original_gdk_clipboard_get_display(clipboard);
    if (
      display != nullptr
      && original_gdk_display_get_primary_clipboard(display) == clipboard
    ) {
      tls_data_set_clipboard(clipboard);
      callback((GObject*)clipboard, nullptr, user_data);
      tls_data_set_clipboard(nullptr);
      return;
    }
  }

  func(
    clipboard,
    cancellable,
    callback,
    user_data
  );
}

static fhh_hook_state_t gdk_clipboard_read_texture_finish_hook_state = {};
static GdkTexture* gdk_clipboard_read_texture_finish_hook(
  GdkClipboard* clipboard,
  GAsyncResult* result,
  GError** error
) {
  FHH_ASSERT_HOOK_SIG_MATCHES(gdk_clipboard_read_texture_finish);
  auto func = FHH_GET_ORIGINAL_FUNC(gdk_clipboard_read_texture_finish);

  if (tls_data_clipboard_matches(clipboard)) {
    if (error != nullptr) {
      *error = nullptr;
    }
    return nullptr;
  }

  return func(
    clipboard,
    result,
    error
  );
}

static fhh_hook_state_t gdk_clipboard_store_async_hook_state = {};
static void gdk_clipboard_store_async_hook(
  GdkClipboard* clipboard,
  int io_priority,
  GCancellable* cancellable,
  GAsyncReadyCallback callback,
  gpointer user_data
) {
  FHH_ASSERT_HOOK_SIG_MATCHES(gdk_clipboard_store_async);
  auto func = FHH_GET_ORIGINAL_FUNC(gdk_clipboard_store_async);

  if (clipboard != nullptr) {
    auto display = original_gdk_clipboard_get_display(clipboard);
    if (
      display != nullptr
      && original_gdk_display_get_primary_clipboard(display) == clipboard
    ) {
      tls_data_set_clipboard(clipboard);
      callback((GObject*)clipboard, nullptr, user_data);
      tls_data_set_clipboard(nullptr);
      return;
    }
  }

  func(
    clipboard,
    io_priority,
    cancellable,
    callback,
    user_data
  );
}

static fhh_hook_state_t gdk_clipboard_store_finish_hook_state = {};
static gboolean gdk_clipboard_store_finish_hook(
  GdkClipboard* clipboard,
  GAsyncResult* result,
  GError** error
) {
  FHH_ASSERT_HOOK_SIG_MATCHES(gdk_clipboard_store_finish);
  auto func = FHH_GET_ORIGINAL_FUNC(gdk_clipboard_store_finish);

  if (tls_data_clipboard_matches(clipboard)) {
    if (error != nullptr) {
      *error = nullptr;
    }
    return true;
  }

  return func(
    clipboard,
    result,
    error
  );
}

static fhh_hook_state_t gdk_clipboard_set_text_hook_state = {};
static void gdk_clipboard_set_text_hook(
  GdkClipboard* clipboard,
  char const* text
) {
  FHH_ASSERT_HOOK_SIG_MATCHES(gdk_clipboard_set_text);
  auto func = FHH_GET_ORIGINAL_FUNC(gdk_clipboard_set_text);

  if (clipboard != nullptr) {
    auto display = original_gdk_clipboard_get_display(clipboard);
    if (
      display != nullptr
      && original_gdk_display_get_primary_clipboard(display) == clipboard
    ) {
      return;
    }
  }

  func(clipboard, text);
}

static fhh_hook_state_t gdk_clipboard_set_texture_hook_state = {};
static void gdk_clipboard_set_texture_hook(
  GdkClipboard* clipboard,
  GdkTexture* texture
) {
  FHH_ASSERT_HOOK_SIG_MATCHES(gdk_clipboard_set_texture);
  auto func = FHH_GET_ORIGINAL_FUNC(gdk_clipboard_set_texture);

  if (clipboard != nullptr) {
    auto display = original_gdk_clipboard_get_display(clipboard);
    if (
      display != nullptr
      && original_gdk_display_get_primary_clipboard(display) == clipboard
    ) {
      return;
    }
  }

  func(clipboard, texture);
}

static fhh_hook_state_t gdk_clipboard_set_value_hook_state = {};
static void gdk_clipboard_set_value_hook(
  GdkClipboard* clipboard,
  GValue const* value
) {
  FHH_ASSERT_HOOK_SIG_MATCHES(gdk_clipboard_set_value);
  auto func = FHH_GET_ORIGINAL_FUNC(gdk_clipboard_set_value);

  if (clipboard != nullptr) {
    auto display = original_gdk_clipboard_get_display(clipboard);
    if (
      display != nullptr
      && original_gdk_display_get_primary_clipboard(display) == clipboard
    ) {
      return;
    }
  }

  func(clipboard, value);
}

static fhh_hook_state_t gdk_clipboard_set_content_hook_state = {};
static gboolean gdk_clipboard_set_content_hook(
  GdkClipboard* clipboard,
  GdkContentProvider* provider
) {
  FHH_ASSERT_HOOK_SIG_MATCHES(gdk_clipboard_set_content);
  auto func = FHH_GET_ORIGINAL_FUNC(gdk_clipboard_set_content);

  if (clipboard != nullptr) {
    auto display = original_gdk_clipboard_get_display(clipboard);
    if (
      display != nullptr
      && original_gdk_display_get_primary_clipboard(display) == clipboard
    ) {
      return true;
    }
  }

  return func(clipboard, provider);
}

static fhh_hook_state_t gdk_clipboard_set_valist_hook_state = {};
static void gdk_clipboard_set_valist_hook(
  GdkClipboard* clipboard,
  GType type,
  va_list args
) {
  FHH_ASSERT_HOOK_SIG_MATCHES(gdk_clipboard_set_valist);
  auto func = FHH_GET_ORIGINAL_FUNC(gdk_clipboard_set_valist);

  if (clipboard != nullptr) {
    auto display = original_gdk_clipboard_get_display(clipboard);
    if (
      display != nullptr
      && original_gdk_display_get_primary_clipboard(display) == clipboard
    ) {
      return;
    }
  }

  func(clipboard, type, args);
}

void hook_gtk4_install_hooks(void* dl_handle) {
  bool installed = false;
  installed |= FHH_INSTALL(dl_handle, gdk_clipboard_read_async);
  installed |= FHH_INSTALL(dl_handle, gdk_clipboard_read_finish);
  installed |= FHH_INSTALL(dl_handle, gdk_clipboard_read_value_async);
  installed |= FHH_INSTALL(dl_handle, gdk_clipboard_read_value_finish);
  installed |= FHH_INSTALL(dl_handle, gdk_clipboard_read_text_async);
  installed |= FHH_INSTALL(dl_handle, gdk_clipboard_read_text_finish);
  installed |= FHH_INSTALL(dl_handle, gdk_clipboard_read_texture_async);
  installed |= FHH_INSTALL(dl_handle, gdk_clipboard_read_texture_finish);
  installed |= FHH_INSTALL(dl_handle, gdk_clipboard_store_async);
  installed |= FHH_INSTALL(dl_handle, gdk_clipboard_store_finish);
  installed |= FHH_INSTALL(dl_handle, gdk_clipboard_set_text);
  installed |= FHH_INSTALL(dl_handle, gdk_clipboard_set_value);
  installed |= FHH_INSTALL(dl_handle, gdk_clipboard_set_texture);
  installed |= FHH_INSTALL(dl_handle, gdk_clipboard_set_content);
  // XXX: gdk_clipboard_set calls _valist internally
  installed |= FHH_INSTALL(dl_handle, gdk_clipboard_set_valist);

  if (installed) {
    initialize_helper_symbols(dl_handle);
  }
}

void hook_gtk4_uninstall_hooks() {
  FHH_UNINSTALL(gdk_clipboard_read_async);
  FHH_UNINSTALL(gdk_clipboard_read_finish);
  FHH_UNINSTALL(gdk_clipboard_read_value_async);
  FHH_UNINSTALL(gdk_clipboard_read_value_finish);
  FHH_UNINSTALL(gdk_clipboard_read_text_async);
  FHH_UNINSTALL(gdk_clipboard_read_text_finish);
  FHH_UNINSTALL(gdk_clipboard_read_texture_async);
  FHH_UNINSTALL(gdk_clipboard_read_texture_finish);
  FHH_UNINSTALL(gdk_clipboard_store_async);
  FHH_UNINSTALL(gdk_clipboard_store_finish);
  FHH_UNINSTALL(gdk_clipboard_set_text);
  FHH_UNINSTALL(gdk_clipboard_set_value);
  FHH_UNINSTALL(gdk_clipboard_set_texture);
  FHH_UNINSTALL(gdk_clipboard_set_content);
  FHH_UNINSTALL(gdk_clipboard_set_valist);
  gdk_clipboard_get_display_func = nullptr;
  gdk_display_get_primary_clipboard_func = nullptr;
}
