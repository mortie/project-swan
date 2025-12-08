executable(
  'swan-launcher',
  'src/main.cc',
  'src/MainWindow.cc',
  'src/system.cc',
  'src/worlds.cc',
  install: true,
  install_rpath: bin_rpath,
  dependencies: [
    libcpptoml,
    libdate,
    libimgui,
    libcygnet,
    libgl,
    libglfw3,
    libstb_image,
    common,
  ],
  cpp_args: [
    '-fPIC',
  ],
)
