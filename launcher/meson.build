libwxwidgets = dependency('wxwidgets', required: false)

if libwxwidgets.found()
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
      libcpptoml,
      libdate,
    ],
  )
endif
