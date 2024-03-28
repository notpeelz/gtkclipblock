project(
  'gtkclipblock',
  'c',
  default_options: [
    'warning_level=3',
    'c_std=c2x',
    'default_library=static',
  ],
)

add_project_arguments([
  '-D_POSIX_C_SOURCE=200809L',
  '-Wno-pedantic',
  '-Wno-error=unused-function',
  '-Wno-error=unused-parameter',
  '-Wno-error=unused-but-set-parameter',
  '-Wno-error=unused-but-set-variable',
  '-Wno-error=unused-variable',
  '-Wno-error=unused-value',
  '-Wno-error=unused-label',
  '-Wno-error=unused-const-variable',
  '-Wno-error=unused-result',
  # HACK: stop vscode's intellisense from complaining about missing c2x features
  '-include', meson.source_root() / 'c2xcompat.h',
], language: 'c')

conf_data = configuration_data()

funchook_helper = subproject('funchook-helper')
funchook = funchook_helper.get_variable('funchook')

funchook_helper_dep = funchook_helper.get_variable('funchook_helper_dep')
distorm_dep = funchook.dependency('distorm')
dl_dep = dependency('dl', include_type: 'system')
threads_dep = dependency('threads', include_type: 'system')

opt_gtk_version = get_option('gtk-version')

gtk2hook_dep = declare_dependency()
opt_gtk2 = get_option('gtk2') \
  .enable_if(opt_gtk_version == '2') \
  .disable_if(opt_gtk_version != '2' and opt_gtk_version != 'hybrid')
if opt_gtk2.allowed()
  conf_data.set('HOOK_GTK2', true)
  gtk2hook_inc = include_directories('src/gtk2/include')
  gtk2hook = static_library(
    meson.project_name() + '_gtk2',
    'src/gtk2/gtk2.c',
    dependencies: [
      funchook_helper_dep,
      distorm_dep,
      dl_dep,
      dependency('gtk+-2.0', include_type: 'system', required: opt_gtk2.enabled())
        .partial_dependency(compile_args: true),
    ],
    include_directories: gtk2hook_inc,
  )
  gtk2hook_dep = declare_dependency(
    link_with: gtk2hook,
    include_directories: gtk2hook_inc,
    dependencies: [],
  )
endif

gtk3hook_dep = declare_dependency()
opt_gtk3 = get_option('gtk3') \
  .enable_if(opt_gtk_version == '3') \
  .disable_if(opt_gtk_version != '3' and opt_gtk_version != 'hybrid')
if opt_gtk3.allowed()
  conf_data.set('HOOK_GTK3', true)
  gtk3hook_inc = include_directories('src/gtk3/include')
  gtk3hook = static_library(
    meson.project_name() + '_gtk3',
    'src/gtk3/gtk3.c',
    dependencies: [
      funchook_helper_dep,
      distorm_dep,
      dl_dep,
      dependency('gtk+-3.0', include_type: 'system', required: opt_gtk3.enabled())
        .partial_dependency(compile_args: true),
    ],
    include_directories: gtk3hook_inc,
  )
  gtk3hook_dep = declare_dependency(
    link_with: gtk3hook,
    include_directories: gtk3hook_inc,
    dependencies: [],
  )
endif

gtk4hook_dep = declare_dependency()
opt_gtk4 = get_option('gtk4') \
  .enable_if(opt_gtk_version == '4') \
  .disable_if(opt_gtk_version != '4' and opt_gtk_version != 'hybrid')
if opt_gtk4.allowed()
  conf_data.set('HOOK_GTK4', true)
  gtk4hook_inc = include_directories('src/gtk4/include')
  gtk4hook = static_library(
    meson.project_name() + '_gtk4',
    'src/gtk4/gtk4.c',
    dependencies: [
      funchook_helper_dep,
      distorm_dep,
      dl_dep,
      dependency('gtk4', include_type: 'system', required: opt_gtk4.enabled())
        .partial_dependency(compile_args: true),
    ],
    include_directories: gtk4hook_inc,
  )
  gtk4hook_dep = declare_dependency(
    link_with: gtk4hook,
    include_directories: gtk4hook_inc,
    dependencies: [],
  )
endif

assert(
  opt_gtk2.allowed() or opt_gtk3.allowed() or opt_gtk4.allowed(),
  'must be configured to hook at least one library'
)

config_h = configure_file(
  output: 'config.h',
  configuration: conf_data
)

soname = meson.project_name()
if opt_gtk_version != 'hybrid'
  soname += '-gtk' + opt_gtk_version
endif

gtkclipblock = shared_library(
  soname,
  'src/main.c',
  install: true,
  dependencies: [
    funchook_helper_dep,
    distorm_dep,
    threads_dep,
    dl_dep,
    gtk2hook_dep,
    gtk3hook_dep,
    gtk4hook_dep,
  ],
  link_args: [
    '-Wl,--version-script', meson.project_source_root() / 'version.map',
  ],
  c_args: [
    '-include', meson.current_build_dir() / 'config.h',
  ],
)
