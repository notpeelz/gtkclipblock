CONF_DATA = configuration_data()

subdir('gtk2')
subdir('gtk3')
subdir('gtk4')

configure_file(
  output: 'config.h',
  configuration: CONF_DATA,
)

shared_library(
  meson.project_name() + get_option('soname-suffix'),
  'main.c',
  install: true,
  dependencies: [
    DEP_FUNCHOOK_HELPER,
    DEP_DISTORM,
    DEP_THREADS,
    DEP_DL,
    DEP_GTK2HOOK,
    DEP_GTK3HOOK,
    DEP_GTK4HOOK,
  ],
  link_args: [
    '-Wl,--version-script=@0@'.format(meson.current_source_dir() / 'version.map'),
  ],
  c_args: [
    '-include', meson.current_build_dir() / 'config.h',
  ],
)
