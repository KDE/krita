set(EXTPREFIX_googleangle "${EXTPREFIX}")

#[[

ANGLE Versioning:

A branch in the format `chromium/<BUILD>` is created in the git repo of ANGLE.
This build number correspond to the build number of Chromium releases. This
can be tracked on OmahaProxy [1]. The latest release regardless of branch
will likely use the latest commit of ANGLE on the corresponding branch.

As of writing (2022-03-16) the latest Beta of Chromium is 100.0.4896.46.
On my end it says in `chrome://gpu` that the ANGLE commit id is `faa96536d88a`.
At the same time, the `chromium/4896` branch is at `b0f33007113d`, which is
actually one commit ahead.

How to update ANGLE:

1. Clone the git repo of ANGLE [2] or fetch updates if already cloned, then
   checkout the current base commit.
2. Apply patches on top of it:
    - `git am path/to/01-patches_qt5.12.12/* path/to/02-patches_krita/*`
    - Check below for actual filenames of the patches.
3. Check the current latest Chromium Beta or Stable build number [1] to decide
   the new branch to use as base.
4. Fetch the latest commit of that branch:
    - `git fetch origin chromium/xxxx`
5. Rebase onto that branch:
    - `git rebase -i --onto origin/chromium/xxxx current_base_commit`
    - Note: Substitute `current_base_commit` with the base commit you started with.
6. Generate new patches:
    - `git format-patch origin/chromium/xxxx -o path/to/output/`
    - `unix2dos path/to/output/*`
    - Then move the updated patches into the corresponding patches dir.
7. Update the details below with the new base commit.
8. Check the `DEPS` file for the latest commit for `third_party/zlib`, then
   update the details for the target `ext_googleangle_zlib` below.

[1]: https://omahaproxy.appspot.com/
[2]: https://chromium.googlesource.com/angle/angle

]]

# ANGLE Revision Information:
#
# Last updated: 2022-05-12
# ANGLE branch: chromium/5005
# ANGLE commit: f2280c0c5f935dccbaf528343d474c8fcdebe63a
# Used by Chromium version: Beta 102.0.5005.49

set(GOOGLE_ANGLE_BASE_COMMIT "f2280c0c5f935dccbaf528343d474c8fcdebe63a")
set(ANGLE_COMMIT_DATE "2022-05-03")

ExternalProject_Add(
    ext_googleangle

    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    DOWNLOAD_NAME angle-${GOOGLE_ANGLE_BASE_COMMIT}.tar.gz
    # URL https://chromium.googlesource.com/angle/angle/+archive/${GOOGLE_ANGLE_BASE_COMMIT}.tar.gz
    # Note: Don't use the archive from googlesource, they don't use deterministic
    #       timestamps so the file changes every time.
    URL https://github.com/google/angle/archive/${GOOGLE_ANGLE_BASE_COMMIT}.tar.gz
    URL_HASH SHA256=9d3ff60560b953dae7d7613b69dda643aef62390574114fdb1c9d4eada672049

    PATCH_COMMAND ${CMAKE_COMMAND} -E echo Patching ANGLE
    # backported fixes from upstream
    COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-D3D-Initialize-storage-after-generating-mipmap-image.patch
    # Patches from Qt 5.12.12:
    COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/01-patches_qt5.12.12/0001-ANGLE-Use-pixel-sizes-in-the-XAML-swap-chain.patch
    COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/01-patches_qt5.12.12/0002-ANGLE-Add-support-for-querying-platform-device.patch
    COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/01-patches_qt5.12.12/0003-ANGLE-Fix-Windows-Store-D3D-Trim-and-Level-9-require.patch
    COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/01-patches_qt5.12.12/0004-ANGLE-fix-usage-of-shared-handles-for-WinRT-applicat.patch
    COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/01-patches_qt5.12.12/0005-ANGLE-Fix-initialization-of-zero-sized-window.patch
    COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/01-patches_qt5.12.12/0006-ANGLE-winrt-Do-full-screen-update-if-the-the-window-.patch
    COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/01-patches_qt5.12.12/0007-Revert-Fix-scanForWantedComponents-not-ignoring-attr.patch
    COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/01-patches_qt5.12.12/0008-ANGLE-Disable-multisampling-to-avoid-crash-in-Qt-app.patch
    COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/01-patches_qt5.12.12/0009-ANGLE-Dynamically-load-D3D-compiler-from-a-list.patch
    COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/01-patches_qt5.12.12/0010-ANGLE-clean-up-displays-on-dll-unload.patch
    # Patches for building with CMake, with CMakeLists.txt referenced from vcpkg and modified:
    # https://github.com/microsoft/vcpkg/blob/5a8a1dfff8e2dffd4c8cc2d1fc0da379ffd18b25/ports/angle/CMakeLists.txt
    COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/02-patches_krita/0011-Fix-MinGW-build.patch
    COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/02-patches_krita/0012-Fix-Clang-MinGW-target-build.patch
    COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/02-patches_krita/0013-Add-CMake-build-file.patch
    COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/02-patches_krita/0014-CMake-Do-not-exclude-EGL-headers.patch
    # Patches for supporting HDR output on Windows:
    COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/02-patches_krita/0015-Implement-openGL-surface-color-space-selection-in-An.patch
    # Miscellaneous fixes:
    COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/02-patches_krita/0016-Reduce-flickering-when-resizing-window.patch
    COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/02-patches_krita/0017-Replace-extern-thread_local-to-avoid-GCC-mingw-w64-b.patch
    # Copy headers:
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/angle_commit.h ./
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/angle_commit.h ./src/common/
    # Copy third_party/zlib:
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_BINARY_DIR}/src-chromium_zlib ${CMAKE_CURRENT_BINARY_DIR}/src/third_party/zlib

    INSTALL_DIR ${EXTPREFIX_googleangle}
    CMAKE_ARGS -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_googleangle} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}
    UPDATE_COMMAND ""

    SOURCE_DIR src
    BINARY_DIR build
    STAMP_DIR stamps
)

# ANGLE requires `third_party/zlib` to be pulled separately. The git commit
# that it expects can be found in the `DEPS` file at the root of the ANGLE
# source tree.
set(GOOGLE_ANGLE_THIRDPARTY_ZLIB_COMMIT "a6d209ab932df0f1c9d5b7dc67cfa74e8a3272c0")
ExternalProject_Add(
    ext_googleangle_zlib

    # DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    # DOWNLOAD_NAME angle-zlib-${GOOGLE_ANGLE_THIRDPARTY_ZLIB_COMMIT}.tar.gz
    # URL https://chromium.googlesource.com/chromium/src/third_party/zlib/+archive/${GOOGLE_ANGLE_THIRDPARTY_ZLIB_COMMIT}.tar.gz
    # URL_HASH SHA256=xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    # Note: Don't use the archive from googlesource, they don't use deterministic
    #       timestamps so the file changes every time.
    GIT_REPOSITORY https://chromium.googlesource.com/chromium/src/third_party/zlib
    GIT_TAG ${GOOGLE_ANGLE_THIRDPARTY_ZLIB_COMMIT}

    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""

    SOURCE_DIR src-chromium_zlib
    STAMP_DIR stamps

    STEP_TARGETS download
)

# This ensures that ANGLE's third_party/zlib will be downloaded before patching ANGLE.
ExternalProject_Add_StepDependencies(ext_googleangle patch ext_googleangle_zlib-download)

# Generate `angle_commit.h`
string(SUBSTRING "${GOOGLE_ANGLE_BASE_COMMIT}" 0 12 ANGLE_COMMIT_HASH)
set(ANGLE_COMMIT_HASH "${ANGLE_COMMIT_HASH}+krita_qt5.12.12")
string(LENGTH "${ANGLE_COMMIT_HASH}" ANGLE_COMMIT_HASH_SIZE)
configure_file(angle_commit.h.in ${CMAKE_CURRENT_BINARY_DIR}/angle_commit.h)
