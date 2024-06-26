project(
  'gtkclipblock',
  'c',
  version: '0.2.4',
  default_options: [
    'warning_level=3',
    'c_std=c2x',
    'default_library=static',
  ],
)

add_project_arguments([
  '-D_DEFAULT_SOURCE',
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

funchook_helper = subproject('funchook-helper')
funchook = funchook_helper.get_variable('funchook')

DEP_FUNCHOOK_HELPER = funchook_helper.get_variable('DEP_FUNCHOOK_HELPER')
DEP_DISTORM = funchook.dependency('distorm')
DEP_DL = dependency('dl', include_type: 'system')
DEP_THREADS = dependency('threads', include_type: 'system')

assert(
  get_option('gtk2').allowed() \
    or get_option('gtk3').allowed() \
    or get_option('gtk4').allowed(),
  'must be configured to hook at least one library'
)

docdir = get_option('docdir')
if docdir == ''
  docdir = join_paths(
    get_option('prefix'),
    get_option('datadir'),
    'doc',
    meson.project_name(),
  )
endif
install_data('README.md', install_dir: docdir)

licensedir = get_option('licensedir')
if licensedir == ''
  licensedir = join_paths(
    get_option('prefix'),
    get_option('datadir'),
    'licenses',
    meson.project_name(),
  )
endif
install_data('LICENSE', install_dir: licensedir)

subdir('src')
