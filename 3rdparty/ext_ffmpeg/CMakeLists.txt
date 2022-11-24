SET(PREFIX_ext_ffmpeg "${EXTPREFIX}")

# ======================== AUDIO ===========================

ExternalProject_Add(
    ext_libogg
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://downloads.xiph.org/releases/ogg/libogg-1.3.5.tar.gz
    URL_HASH SHA256=0eb4b4b9420a0f51db142ba3f9c64b333f826532dc0f48c6410ae51f4799b664
 
    CMAKE_ARGS ${LIBOGG_EXTRA_CMAKE_ARGS} -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_ffmpeg} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}
)

ExternalProject_Add(
    ext_libvorbis
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://downloads.xiph.org/releases/vorbis/libvorbis-1.3.7.tar.xz
    URL_HASH SHA256=b33cc4934322bcbf6efcbacf49e3ca01aadbea4114ec9589d1b1e9d20f72954b

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-vorbis-Fix-library-definition-files-lacking-a-library.patch

    CMAKE_ARGS ${LIBVORBIS_EXTRA_CMAKE_ARGS} -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_ffmpeg} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    DEPENDS ${OGG_DEP}
)

ExternalProject_Add(
    ext_flac
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/xiph/flac/releases/download/1.4.2/flac-1.4.2.tar.xz
    URL_HASH SHA256=e322d58a1f48d23d9dd38f432672865f6f79e73a6f9cc5a5f57fcaa83eb5a8e4

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_ffmpeg}
        -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
        -DBUILD_SHARED_LIBS=TRUE
        -DBUILD_EXAMPLES=FALSE
        -DBUILD_TESTING=FALSE
        -DBUILD_DOCS=FALSE
        -DINSTALL_MANPAGES=FALSE
        ${GLOBAL_PROFILE}

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
        ${GLOBAL_PROFILE}
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

    DEPENDS ${MESON_DEP} ${PKG_CONFIG_DEP} ext_libmp3lame_meson
)

ExternalProject_Add(
    ext_sdl2
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/libsdl-org/SDL/releases/download/release-2.26.0/SDL2-2.26.0.tar.gz
    URL_HASH SHA256=8000d7169febce93c84b6bdf376631f8179132fd69f7015d4dadb8b9c2bdb295

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_ffmpeg}
        -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
        -DBUILD_SHARED_LIBS=TRUE
        ${GLOBAL_PROFILE}
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
    GIT_TAG meson-4.4.3
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
            -Dlibvpx_vp9_encoder=enabled
            -Dlibvpx_vp8_encoder=enabled
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

    DEPENDS ${FLAC_DEP} ${VPX_DEP} ${OPUS_DEP} ${SDL_DEP} ext_libvorbis ext_libopenh264 ext_libmp3lame ext_libaom ext_webp ext_png ext_jpeg ${MESON_DEP} ${PKG_CONFIG_DEP}
)
