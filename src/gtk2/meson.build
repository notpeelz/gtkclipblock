DEP_GTK2HOOK = declare_dependency()
if get_option('gtk2').allowed()
  CONF_DATA.set('HOOK_GTK2', true)
  inc = include_directories('.')
  lib = static_library(
    meson.project_name() + '_gtk2',
    'gtk2.c',
    dependencies: [
      DEP_FUNCHOOK_HELPER,
      DEP_DISTORM,
      DEP_DL,
      dependency('gtk+-2.0', include_type: 'system', required: true)
        .partial_dependency(compile_args: true),
    ],
    include_directories: inc,
  )
  DEP_GTK2HOOK = declare_dependency(
    link_with: lib,
    include_directories: inc,
    dependencies: [],
  )
endif
