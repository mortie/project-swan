# SDL_net sadly can't just be used as a cmake subproject,
# because one cmake subproject can't depend on another cmake subproject

libsdl_net = declare_dependency(
  include_directories: 'SDL_net/include',
  link_with: shared_library(
    'SDL_net',
    'SDL_net/src/SDL_net.c',
    include_directories: 'SDL_net/include',
    c_args: [
      '-DSDL_PLATFORM_' + sdl_platform,
    ],
    install: true,
    dependencies: [libsdl],
    install_rpath: origin_rpath,
  )
)
