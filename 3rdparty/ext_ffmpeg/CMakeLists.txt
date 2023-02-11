SET(PREFIX_ext_ffmpeg "${EXTPREFIX}")

# ======================== AUDIO ===========================

ExternalProject_Add(
    ext_libogg
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://downloads.xiph.org/releases/ogg/libogg-1.3.5.tar.gz
    URL_HASH SHA256=0eb4b4b9420a0f51db142ba3f9c64b333f826532dc0f48c6410ae51f4799b664
 
    CMAKE_ARGS ${LIBOGG_EXTRA_CMAKE_ARGS} -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_ffmpeg} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)

ExternalProject_Add(
    ext_libvorbis
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://downloads.xiph.org/releases/vorbis/libvorbis-1.3.7.tar.xz
    URL_HASH SHA256=b33cc4934322bcbf6efcbacf49e3ca01aadbea4114ec9589d1b1e9d20f72954b

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-vorbis-Fix-library-definition-files-lacking-a-library.patch

    CMAKE_ARGS ${LIBVORBIS_EXTRA_CMAKE_ARGS} -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_ffmpeg} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""

    DEPENDS ${OGG_DEP}
)

ExternalProject_Add(
    ext_libtheora
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    GIT_REPOSITORY https://gitlab.xiph.org/xiph/theora.git
    # gitlab-ci: Add scons build. Latest commit as of Nov 26, 2022
    GIT_TAG 7180717276af1ebc7da15c83162d6c5d6203aabf

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-theora-Add-Meson-build-system.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-theora-Add-support-for-hidden-frame-dumping-flag.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0003-theora-Add-missing-libtheora-module-definition-file.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0004-theora-Add-tests.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0005-theora-Add-examples.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0006-theora-Add-telemetry-support.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0007-theora-Add-memory-constraint-fuzzer.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0008-theora-Add-specification-building.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0009-theora-Add-doxygen-support.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0010-theora-Update-to-1.2.0.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0011-theora-Add-Arm-support.patch

    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        PKG_CONFIG_ALLOW_SYSTEM_CFLAGS=1
        ${MESON_BINARY_PATH} setup <BINARY_DIR> <SOURCE_DIR>
            --prefix=${PREFIX_ext_ffmpeg}
            --libdir=lib
            -Ddoc=disabled
            -Dspec=disabled
            ${EXTRA_MESON_FLAGS}
    BUILD_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        ${MESON_BINARY_PATH} compile -C <BINARY_DIR> -j${SUBMAKE_JOBS}
    INSTALL_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        ${MESON_BINARY_PATH} install -C <BINARY_DIR>

    UPDATE_COMMAND ""

    DEPENDS ${MESON_DEP} ${PKG_CONFIG_DEP} ${OGG_DEP} ext_libvorbis
)

ExternalProject_Add(
    ext_flac
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    # the official .tar.xz cannot be unpacked by CMake on macOS due to usage
    # of UTF-8 internationalized filenames in
    # test/flac-to-flac-metadata-test-files.
    GIT_REPOSITORY https://github.com/xiph/flac.git
    GIT_TAG 1.4.2

    # Emmet: if you need metaflac, please add:
    # -DIconv_IS_BUILT_IN=TRUE
    # -DBUILD_PROGRAMS=FALSE
    # and add ${ICONV_DEP} to DEPENDS.
    # otherwise CMake's FindIconv gets confused due to libiconv having
    # its headers in the include path

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_ffmpeg}
        -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
        -DBUILD_SHARED_LIBS=TRUE
        -DBUILD_PROGRAMS=FALSE
        -DBUILD_EXAMPLES=FALSE
        -DBUILD_TESTING=FALSE
        -DBUILD_DOCS=FALSE
        -DINSTALL_MANPAGES=FALSE
        ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""

    DEPENDS ${OGG_DEP}
)

ExternalProject_Add(
    ext_opus
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    # Official tarball has a missing opus_buildtype.cmake
    # The Git tag is complete
    GIT_REPOSITORY https://gitlab.xiph.org/xiph/opus.git
    GIT_TAG v1.3.1

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_ffmpeg}
        -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
        -DBUILD_SHARED_LIBS=TRUE
        -DOPUS_STACK_PROTECTOR=OFF
        ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)

# ======================== VIDEO ===========================

ExternalProject_Add(
    ext_libopenh264
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/cisco/openh264/archive/v2.3.1.tar.gz
    URL_HASH SHA256=453afa66dacb560bc5fd0468aabee90c483741571bca820a39a1c07f0362dc32

    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        PKG_CONFIG_ALLOW_SYSTEM_CFLAGS=1
        ${MESON_BINARY_PATH} setup <BINARY_DIR> <SOURCE_DIR>
            --prefix=${PREFIX_ext_ffmpeg}
            --libdir=lib
            -Dbuildtype=$<IF:$<CONFIG:Debug>,debug,debugoptimized>
            -Dtests=disabled
            ${EXTRA_MESON_FLAGS}
    BUILD_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        ${MESON_BINARY_PATH} compile -C <BINARY_DIR> -j${SUBMAKE_JOBS}
    INSTALL_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        ${MESON_BINARY_PATH} install -C <BINARY_DIR>

    UPDATE_COMMAND ""

    DEPENDS ext_nasm ${MESON_DEP} ${PKG_CONFIG_DEP}
)

ExternalProject_Add(
    ext_libmp3lame_meson
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://wrapdb.mesonbuild.com/v2/lame_3.100-5/get_patch
    URL_HASH SHA256=ed9d897e8ce69e521551812b232de0a651da710d6b0f91606cad4e3afdfd137f

    DOWNLOAD_NAME "lame_3.100-5_patch.zip"

    BUILD_COMMAND ""
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""

    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/meson"
)

ExternalProject_Add(
    ext_libmp3lame
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://sourceforge.net/projects/lame/files/lame/3.100/lame-3.100.tar.gz
    URL_HASH SHA256=ddfe36cab873794038ae2c1210557ad34857a4b6bdc515785d1da9e175b1da1e

    PATCH_COMMAND ${CMAKE_COMMAND} -E echo Deploying Meson wrap
        COMMAND ${CMAKE_COMMAND} -E copy_directory 
            ${CMAKE_CURRENT_BINARY_DIR}/meson
            <SOURCE_DIR>
        COMMAND ${CMAKE_COMMAND} -E echo Deploying patches
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-lame-Add-SOVERSION-to-the-library-for-disambiguation.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-lame-Install-pkg-config-file.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0003-lame-Add-DLL-version-information.patch

    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        PKG_CONFIG_ALLOW_SYSTEM_CFLAGS=1
        ${MESON_BINARY_PATH} setup <BINARY_DIR> <SOURCE_DIR>
            --prefix=${PREFIX_ext_ffmpeg}
            --libdir=lib
            -Dbuildtype=$<IF:$<CONFIG:Debug>,debug,debugoptimized>
            ${EXTRA_MESON_FLAGS}

    BUILD_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        ${MESON_BINARY_PATH} compile -C <BINARY_DIR> -j${SUBMAKE_JOBS}

    INSTALL_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        ${MESON_BINARY_PATH} install -C <BINARY_DIR>

    UPDATE_COMMAND ""

    DEPENDS ${MESON_DEP} ${PKG_CONFIG_DEP} ext_libmp3lame_meson
)

if (NOT APPLE)
    ExternalProject_Add(
        ext_sdl2
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/libsdl-org/SDL/releases/download/release-2.26.0/SDL2-2.26.0.tar.gz
        URL_HASH SHA256=8000d7169febce93c84b6bdf376631f8179132fd69f7015d4dadb8b9c2bdb295

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_ffmpeg}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
            -DBUILD_SHARED_LIBS=TRUE
            ${GLOBAL_PROFILE}

        UPDATE_COMMAND ""
    )
else()
    ExternalProject_Add(
        ext_sdl2
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/libsdl-org/SDL/releases/download/release-2.26.0/SDL2-2.26.0.tar.gz
        URL_HASH SHA256=8000d7169febce93c84b6bdf376631f8179132fd69f7015d4dadb8b9c2bdb295

        CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_ffmpeg}
            --without-x
            ${GLOBAL_AUTOMAKE_PROFILE}

        UPDATE_COMMAND ""
    )
endif()

# TODO: migrate to a proper cross-platform wrap.
ExternalProject_Add(
    ext_vpx
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/webmproject/libvpx/archive/v1.12.0.tar.gz
    URL_HASH SHA256=f1acc15d0fd0cb431f4bf6eac32d5e932e40ea1186fe78e074254d6d003957bb

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-MacOS-13-is-darwin22.patch

    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_ffmpeg}  ${GLOBAL_AUTOMAKE_PROFILE}

    UPDATE_COMMAND ""
)

# IMPORTANT:
# OpenSUSE's list of whitelisted encoders: https://build.opensuse.org/package/view_file/openSUSE:Factory/ffmpeg-5/enable_encoders
# OpenSUSE's list of whitelisted decoders: https://build.opensuse.org/package/view_file/openSUSE:Factory/ffmpeg-5/enable_decoders
# Above are the lists of codecs that have been approved for use by OpenSUSE's legal department.
# Anything not enabled (missing or commented out) in these lists is considered unsafe and should NOT be used!
# While unlikely, it's possible that these lists could change, so it's worth occasionally cross-referencing.

ExternalProject_Add(
    ext_ffmpeg
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    GIT_REPOSITORY https://gitlab.freedesktop.org/gstreamer/meson-ports/ffmpeg.git
    GIT_TAG meson-4.4

    # Workaround https://github.com/mesonbuild/meson/issues/11113
    # See https://gitlab.freedesktop.org/gstreamer/meson-ports/ffmpeg/-/issues/31
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-meson-Fix-library-detection-in-prefixed-builds.patch

    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        PKG_CONFIG_ALLOW_SYSTEM_CFLAGS=1
        ${MESON_BINARY_PATH} setup <BINARY_DIR> <SOURCE_DIR>
            --prefix=${PREFIX_ext_ffmpeg}
            --libdir=lib
            -Dbuildtype=$<IF:$<CONFIG:Debug>,debug,debugoptimized>
            -Ddefault_library=shared
            -Dcli=enabled
            -Dtests=disabled
            -Dffprobe=disabled
            -Dffplay=enabled
            -Davdevice=disabled
            -Davcodec=enabled
            -Davformat=enabled
            -Davutil=enabled
            -Davresample=enabled
            -Dswresample=enabled
            -Dswscale=enabled
            -Dpostproc=disabled
            -Dsdl2=enabled
            -Dlibaom=enabled
            -Dimage2_demuxer=enabled
            -Dimage2_alias_pix_demuxer=enabled
            -Dimage2_brender_pix_demuxer=enabled
            # ===================== Container Formats
            -Dwebm_muxer=enabled
            -Dmatroska_muxer=enabled
            -Dogg_muxer=enabled
            # ===================== Image Formats
            -Dlibwebp=enabled
            -Dpng_encoder=enabled
            -Dpng_decoder=enabled
            -Dapng_encoder=enabled
            -Dapng_decoder=enabled
            # ===================== Video Codecs
            -Dlibaom_av1_encoder=enabled
            -Dlibvpx_vp9_encoder=auto
            -Dlibvpx_vp8_encoder=auto
            -Dlibopenh264=enabled
            -Dlibtheora=enabled
            # ===================== Audio Codecs
            -Dflac_encoder=enabled
            -Daac_encoder=enabled
            -Dlibmp3lame=enabled
            -Dlibopus=enabled
            -Dlibvorbis=enabled
            ${EXTRA_MESON_FLAGS}

    BUILD_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        ${MESON_BINARY_PATH} compile -C <BINARY_DIR> -j${SUBMAKE_JOBS}

    INSTALL_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        ${MESON_BINARY_PATH} install -C <BINARY_DIR>

    UPDATE_COMMAND ""

    DEPENDS ${FLAC_DEP} ${VPX_DEP} ${OPUS_DEP} ${SDL_DEP} ${VPX_DEP} ext_libvorbis ext_libtheora ext_libopenh264 ext_libmp3lame ext_libaom ext_webp ext_png ext_jpeg ${MESON_DEP} ${PKG_CONFIG_DEP}
)
