#include <assert.h>
#include <dlfcn.h>
#include <gtk/gtk.h>
#include <funchook-helper.h>
#include "gtk3.h"

static typeof(&gtk_clipboard_get_selection) gtk_clipboard_get_selection_func = nullptr;

static void initialize_helper_symbols(void* handle) {
  gtk_clipboard_get_selection_func =
    (typeof(&gtk_clipboard_get_selection))dlsym(handle, "gtk_clipboard_get_selection");
  assert(gtk_clipboard_get_selection_func != nullptr);
}

static GdkAtom original_gtk_clipboard_get_selection(GtkClipboard* clipboard) {
  assert(gtk_clipboard_get_selection_func != nullptr);
  return gtk_clipboard_get_selection_func(clipboard);
}

static fhh_hook_state_t gtk_clipboard_set_with_data_hook_state = {};
static gboolean gtk_clipboard_set_with_data_hook(
  GtkClipboard* clipboard,
  GtkTargetEntry const* targets,
  guint n_targets,
  GtkClipboardGetFunc get_func,
  GtkClipboardClearFunc clear_func,
  gpointer user_data
) {
  FHH_ASSERT_HOOK_SIG_MATCHES(gtk_clipboard_set_with_data);
  auto func = FHH_GET_ORIGINAL_FUNC(gtk_clipboard_set_with_data);

  if (
    clipboard != nullptr
    && original_gtk_clipboard_get_selection(clipboard) == GDK_SELECTION_PRIMARY
  ) {
    return true;
  }

  return func(
    clipboard,
    targets,
    n_targets,
    get_func,
    clear_func,
    user_data
  );
}

static fhh_hook_state_t gtk_clipboard_set_with_owner_hook_state = {};
static gboolean gtk_clipboard_set_with_owner_hook(
  GtkClipboard* clipboard,
  GtkTargetEntry const* targets,
  guint n_targets,
  GtkClipboardGetFunc get_func,
  GtkClipboardClearFunc clear_func,
  GObject* owner
) {
  FHH_ASSERT_HOOK_SIG_MATCHES(gtk_clipboard_set_with_owner);
  auto func = FHH_GET_ORIGINAL_FUNC(gtk_clipboard_set_with_owner);

  if (
    clipboard != nullptr
    && original_gtk_clipboard_get_selection(clipboard) == GDK_SELECTION_PRIMARY
  ) {
    return true;
  }

  return func(
    clipboard,
    targets,
    n_targets,
    get_func,
    clear_func,
    owner
  );
}

static fhh_hook_state_t gtk_clipboard_set_text_hook_state = {};
static void gtk_clipboard_set_text_hook(
  GtkClipboard* clipboard,
  gchar const* text,
  gint len
) {
  FHH_ASSERT_HOOK_SIG_MATCHES(gtk_clipboard_set_text);
  auto func = FHH_GET_ORIGINAL_FUNC(gtk_clipboard_set_text);

  if (
    clipboard != nullptr
    && original_gtk_clipboard_get_selection(clipboard) == GDK_SELECTION_PRIMARY
  ) {
    return;
  }

  return func(
    clipboard,
    text,
    len
  );
}

static fhh_hook_state_t gtk_clipboard_set_image_hook_state = {};
static void gtk_clipboard_set_image_hook(
  GtkClipboard* clipboard,
  GdkPixbuf* pixbuf
) {
  FHH_ASSERT_HOOK_SIG_MATCHES(gtk_clipboard_set_image);
  auto func = FHH_GET_ORIGINAL_FUNC(gtk_clipboard_set_image);

  if (
    clipboard != nullptr
    && original_gtk_clipboard_get_selection(clipboard) == GDK_SELECTION_PRIMARY
  ) {
    return;
  }

  return func(
    clipboard,
    pixbuf
  );
}

static fhh_hook_state_t gtk_clipboard_set_can_store_hook_state = {};
static void gtk_clipboard_set_can_store_hook(
  GtkClipboard* clipboard,
  GtkTargetEntry const* targets,
  gint n_targets
) {
  FHH_ASSERT_HOOK_SIG_MATCHES(gtk_clipboard_set_can_store);
  auto func = FHH_GET_ORIGINAL_FUNC(gtk_clipboard_set_can_store);

  if (
    clipboard != nullptr
    && original_gtk_clipboard_get_selection(clipboard) == GDK_SELECTION_PRIMARY
  ) {
    return;
  }

  return func(
    clipboard,
    targets,
    n_targets
  );
}

static fhh_hook_state_t gtk_clipboard_store_hook_state = {};
static void gtk_clipboard_store_hook(GtkClipboard* clipboard) {
  FHH_ASSERT_HOOK_SIG_MATCHES(gtk_clipboard_store);
  auto func = FHH_GET_ORIGINAL_FUNC(gtk_clipboard_store);

  if (
    clipboard != nullptr
    && original_gtk_clipboard_get_selection(clipboard) == GDK_SELECTION_PRIMARY
  ) {
    return;
  }

  return func(clipboard);
}

static fhh_hook_state_t gtk_clipboard_request_contents_hook_state = {};
static void gtk_clipboard_request_contents_hook(
  GtkClipboard* clipboard,
  GdkAtom target,
  GtkClipboardReceivedFunc callback,
  gpointer user_data
) {
  FHH_ASSERT_HOOK_SIG_MATCHES(gtk_clipboard_request_contents);
  auto func = FHH_GET_ORIGINAL_FUNC(gtk_clipboard_request_contents);

  if (
    clipboard != nullptr
    && original_gtk_clipboard_get_selection(clipboard) == GDK_SELECTION_PRIMARY
  ) {
    typedef struct {
      GdkAtom selection;
      GdkAtom target;
      GdkAtom type;
      gint format;
      guchar* data;
      gint length;
      GdkDisplay* display;
    } private_GtkSelectionData_t;
    static private_GtkSelectionData_t selection_data = {
      .length = -1,
    };
    callback(clipboard, (GtkSelectionData*)&selection_data, user_data);
    return;
  }

  func(clipboard, target, callback, user_data);
}

void hook_gtk3_install_hooks(void* dl_handle) {
  bool installed = false;
  installed |= FHH_INSTALL(dl_handle, gtk_clipboard_set_with_data);
  installed |= FHH_INSTALL(dl_handle, gtk_clipboard_set_with_owner);
  installed |= FHH_INSTALL(dl_handle, gtk_clipboard_set_text);
  installed |= FHH_INSTALL(dl_handle, gtk_clipboard_set_image);
  installed |= FHH_INSTALL(dl_handle, gtk_clipboard_set_can_store);
  installed |= FHH_INSTALL(dl_handle, gtk_clipboard_store);
  installed |= FHH_INSTALL(dl_handle, gtk_clipboard_request_contents);

  if (installed) {
    initialize_helper_symbols(dl_handle);
  }
}

void hook_gtk3_uninstall_hooks() {
  FHH_UNINSTALL(gtk_clipboard_set_with_data);
  FHH_UNINSTALL(gtk_clipboard_set_with_owner);
  FHH_UNINSTALL(gtk_clipboard_set_text);
  FHH_UNINSTALL(gtk_clipboard_set_image);
  FHH_UNINSTALL(gtk_clipboard_set_can_store);
  FHH_UNINSTALL(gtk_clipboard_store);
  FHH_UNINSTALL(gtk_clipboard_request_contents);
  gtk_clipboard_get_selection_func = nullptr;
}
