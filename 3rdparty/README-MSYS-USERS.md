# Building Krita with MSYS2

**Please note that this is an unsupported, best-effort configuration.
Use it at your own risk.**

## Requirements

Use the MINGW64, CLANG64 or UCRT64 flavors. Install the following components
(change the `x86_64` architecture to your own):

- Your flavor's compiler:
  - `pacman -S mingw-w64-x86_64-toolchain`
  - `pacman -S mingw-w64-clang-x86_64-toolchain`
  - `pacman -S mingw-w64-ucrt-x86_64-toolchain`

- CMake:
  - `pacman -S mingw-w64-x86_64-cmake`
  - `pacman -S mingw-w64-clang-x86_64-cmake`
  - `pacman -S mingw-w64-ucrt-x86_64-cmake`

If using the CLANG64 flavor, set the `CC` and `CXX` environment variables:

```
set CC=x86_64-w64-mingw32-cc
set CXX=x86_64-w64-mingw32-c++
```

If you want AVIF/HEIF support, before continuing, install:
  - Rust:
    - **Stock Rust is not recommended with UCRT64 and CLANG64. Use the MSYS2-provided binaries.**
    - `pacman -S mingw-w64-x86_64-rust`
    - `pacman -S mingw-w64-clang-x86_64-cmake`
    - `pacman -S mingw-w64-ucrt-x86_64-cmake`
  - Meson and Ninja:
    - Issue `pip install -U meson setuptools ninja` on your Python installation, **not** on your MSYS2 shell.
    - For more information, see the README.md in this directory, section "Prerequisites", item 4.

Now follow the README.md document, Prerequisites section, item 4 onwards.
Alternatively, run the `build-tools\windows\build.cmd` script from a command 
line that can use the MSYS2 tools.
