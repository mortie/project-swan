# Project: SWAN

[![Check](https://github.com/mortie/project-swan/actions/workflows/build.yml/badge.svg)](https://github.com/mortie/project-swan/actions/workflows/build.yml)

Project: SWAN is (or, will become) a game about exploring a procedurally generated
tile-based world and building contraptions in it to automate stuff.
I have lots of cool ideas, most of them unimplemented.

![screenshot](https://raw.githubusercontent.com/mortie/project-swan/main/screenshot.png)

## Compiling on UNIX

Install dependencies:

### Fedora

```shell
sudo dnf install \
    git make meson cmake gcc g++ glfw-devel capnp capnproto-devel zlib-devel wxGTK-devel \
    libavcodec-free-devel libavutil-free-devel libswscale-free-devel libavformat-free-devel
```

### Ubuntu

```shell
sudo apt install \
    git make meson cmake build-essential pkg-config libglfw3-dev capnproto libcapnp-dev \
    libz-dev libwxgtk3.2-dev libavcodec-dev libavutil-dev libswscale-dev libavformat-dev
```

### macOS

(First, install Homebrew from https://brew.sh/

```shell
brew install meson cmake pkg-config glfw3 capnp wxwidgets ffmpeg portaudio
```

---

Download the source with:

```
git clone https://github.com/mortie/project-swan.git
cd project-swan
git submodule update --init
```

Then compile:

```
make
```

Run the launcher with:

```
make launcher
```
