DEP_GTK3HOOK = declare_dependency()
if get_option('gtk3').allowed()
  CONF_DATA.set('HOOK_GTK3', true)
  inc = include_directories('.')
  lib = static_library(
    meson.project_name() + '_gtk3',
    'gtk3.c',
    dependencies: [
      DEP_FUNCHOOK_HELPER,
      DEP_DISTORM,
      DEP_DL,
      dependency('gtk+-3.0', include_type: 'system', required: true)
        .partial_dependency(compile_args: true),
    ],
    include_directories: inc,
  )
  DEP_GTK3HOOK = declare_dependency(
    link_with: lib,
    include_directories: inc,
    dependencies: [],
  )
endif
