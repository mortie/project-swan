libwxwidgets = dependency('wxwidgets')

executable(
  'swan-launcher',
  'src/main.cc',
  'src/MainWindow.cc',
  'src/SwanLauncher.cc',
  'src/system.cc',
  'src/worlds.cc',
  install: true,
  install_rpath: get_option('libdir'),
  dependencies: [
    libwxwidgets,
  ] + maybe_libtracy,
)

