DEP_GTK4HOOK = declare_dependency()
if get_option('gtk4').allowed()
  CONF_DATA.set('HOOK_GTK4', true)
  inc = include_directories('.')
  lib = static_library(
    meson.project_name() + '_gtk4',
    'gtk4.c',
    dependencies: [
      DEP_FUNCHOOK_HELPER,
      DEP_DISTORM,
      DEP_DL,
      dependency('gtk4', include_type: 'system', required: true)
        .partial_dependency(compile_args: true),
    ],
    include_directories: inc,
  )
  DEP_GTK4HOOK = declare_dependency(
    link_with: lib,
    include_directories: inc,
    dependencies: [],
  )
endif
