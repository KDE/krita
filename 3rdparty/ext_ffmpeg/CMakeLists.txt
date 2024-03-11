SET(PREFIX_ext_ffmpeg "${EXTPREFIX}")

# ======================== AUDIO ===========================

ExternalProject_Add(
    ext_libogg
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://downloads.xiph.org/releases/ogg/libogg-1.3.5.tar.gz
    URL_HASH SHA256=0eb4b4b9420a0f51db142ba3f9c64b333f826532dc0f48c6410ae51f4799b664

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-libogg-Fix-library-filename-on-Windows.patch

    CMAKE_ARGS ${LIBOGG_EXTRA_CMAKE_ARGS} -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_ffmpeg} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)

ExternalProject_Add(
    ext_libvorbis
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://downloads.xiph.org/releases/vorbis/libvorbis-1.3.7.tar.xz
    URL_HASH SHA256=b33cc4934322bcbf6efcbacf49e3ca01aadbea4114ec9589d1b1e9d20f72954b

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-libvorbis-Fix-empty-LIBRARY-tag-in-.def-files.patch

    CMAKE_ARGS ${LIBVORBIS_EXTRA_CMAKE_ARGS} -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_ffmpeg} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""

    DEPENDS ${OGG_DEP}
)

ExternalProject_Add(
    ext_libtheora_meson
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://wrapdb.mesonbuild.com/v2/theora_1.1.1-4/get_patch
    URL_HASH SHA256=210af99fbc5a1b34d0f32270c25ecfb6fae7e76c1ae9dc6971b063317c97a826

    DOWNLOAD_NAME "theora_1.1.1-4_patch.zip"

    BUILD_COMMAND ""
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""

    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/theora_meson"
)

kis_ExternalProject_Add_with_separate_builds_apple(
    ext_libtheora
    MESON

    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    GIT_REPOSITORY https://gitlab.xiph.org/xiph/theora.git
    # gitlab-ci: Add scons build. Latest commit as of Nov 26, 2022
    GIT_TAG 7180717276af1ebc7da15c83162d6c5d6203aabf

    PATCH_COMMAND ${CMAKE_COMMAND} -E echo Deploying Meson wrap
        COMMAND ${CMAKE_COMMAND} -E copy_directory 
            ${CMAKE_CURRENT_BINARY_DIR}/theora_meson
            <SOURCE_DIR>
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0010-theora-Update-to-1.2.0.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0011-theora-Add-Arm-support.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-libtheora-Fix-build-on-android-x86.patch

    CONFIGURE_ARGS --prefix=${PREFIX_ext_ffmpeg}
        -Ddoc=disabled
        -Dspec=disabled
        ${EXTRA_MESON_FLAGS}

    UPDATE_COMMAND ""

    DEPENDS ${MESON_DEP} ${PKG_CONFIG_DEP} ${OGG_DEP} ext_png ext_libvorbis ext_libtheora_meson
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

kis_ExternalProject_Add_with_separate_builds_apple(
    ext_libopenh264
    MESON

    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    DOWNLOAD_NAME openh264-v2.3.1.tar.gz
    URL https://github.com/cisco/openh264/archive/v2.3.1.tar.gz
    URL_HASH SHA256=453afa66dacb560bc5fd0468aabee90c483741571bca820a39a1c07f0362dc32

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-libopenh264-Enable-PIC-for-the-assembler-code-on-x86.patch

    CONFIGURE_ARGS --prefix=${PREFIX_ext_ffmpeg}
            -Dtests=disabled
            ${EXTRA_MESON_FLAGS}

    DEPENDS ext_nasm ${MESON_DEP} ${PKG_CONFIG_DEP}
)


ExternalProject_Add(
    ext_libmp3lame_meson
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://wrapdb.mesonbuild.com/v2/lame_3.100-9/get_patch
    URL_HASH SHA256=bedda639a621b2046fb04110a51870d6c9640f2f4282b5d877bc452e9943c22e

    DOWNLOAD_NAME "lame_3.100-9_patch.zip"

    BUILD_COMMAND ""
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""

    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/lame_meson"
)

kis_ExternalProject_Add_with_separate_builds_apple(
    ext_libmp3lame
    MESON

    URL https://sourceforge.net/projects/lame/files/lame/3.100/lame-3.100.tar.gz
    URL_HASH SHA256=ddfe36cab873794038ae2c1210557ad34857a4b6bdc515785d1da9e175b1da1e

    PATCH_COMMAND ${CMAKE_COMMAND} -E echo Deploying Meson wrap
        COMMAND ${CMAKE_COMMAND} -E copy_directory 
            ${CMAKE_CURRENT_BINARY_DIR}/lame_meson
            <SOURCE_DIR>
        COMMAND ${CMAKE_COMMAND} -E echo Deploying patches
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-lame-Install-pkg-config-file.patch

    CONFIGURE_ARGS --prefix=${PREFIX_ext_ffmpeg}
            ${EXTRA_MESON_FLAGS}

    DEPENDS ${MESON_DEP} ${PKG_CONFIG_DEP} ext_libmp3lame_meson
)

if (ANDROID)
    ExternalProject_Add(
        ext_sdl2
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        GIT_REPOSITORY https://github.com/sh-zam/SDL-android-glue.git
        GIT_TAG 8babf290ece99a58a5354d52775bf79063cf7cf5

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_ffmpeg}
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
            -DBUILD_SHARED_LIBS=TRUE -DSDL_ANDROID_AUDIO_STRIPPED=ON -DSDL_HAPTIC=OFF
            -DSDL_JOYSTICK=OFF -DSDL_VIDEO=OFF -DSDL_SENSOR=OFF -DSDL_MISC=OFF -DSDL_POWER=OFF
            -DSDL_LOCALE=OFF -DSDL_FILESYSTEM=OFF -DINSTALL_JARS_PATH=${PREFIX_ext_ffmpeg}/jar
            -DANDROID_SDK=${ANDROID_SDK_ROOT} -DANDROID_JAVA_PLATFORM_API=${ANDROID_SDK_COMPILE_API}
            ${GLOBAL_PROFILE}


        UPDATE_COMMAND ""
    )
else()
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
endif()


if (ANDROID AND ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
    set (_extra_meson_flags_orig  ${EXTRA_MESON_FLAGS})
    set (EXTRA_MESON_FLAGS -Dmultithread=disabled "${EXTRA_MESON_FLAGS}")
endif()

kis_ExternalProject_Add_with_separate_builds_apple(
    ext_vpx
    MESON

    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    GIT_REPOSITORY https://github.com/dimula73/libvpx.git
    GIT_TAG meson

    WORKING_DIRECTORY <BINARY_DIR>

    CONFIGURE_ARGS --prefix=${PREFIX_ext_ffmpeg}
            -Ddefault_library=shared
            -Dunit_tests=disabled
            -Dtools=disabled
            -Ddocs=disabled
            ${EXTRA_MESON_FLAGS}

    UPDATE_COMMAND ""

    DEPENDS ext_nasm ${MESON_DEP} ${PKG_CONFIG_DEP}
)

if (ANDROID AND ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
    set (EXTRA_MESON_FLAGS ${_extra_meson_flags_orig})
    set(DISABLE_ASM_ON_ANDROID_x86_32 "-Dasm=disabled" "-Dinline_asm=disabled")
endif()
# IMPORTANT:
# OpenSUSE's list of whitelisted encoders: https://build.opensuse.org/package/view_file/openSUSE:Factory/ffmpeg-5/enable_encoders
# OpenSUSE's list of whitelisted decoders: https://build.opensuse.org/package/view_file/openSUSE:Factory/ffmpeg-5/enable_decoders
# Above are the lists of codecs that have been approved for use by OpenSUSE's legal department.
# Anything not enabled (missing or commented out) in these lists is considered unsafe and should NOT be used!
# While unlikely, it's possible that these lists could change, so it's worth occasionally cross-referencing.

if (NOT MSVC)
    set(_stack_guard_flag "-Denable-ssp=enabled")
endif() 

kis_ExternalProject_Add_with_separate_builds_apple(
    ext_ffmpeg
    MESON
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    GIT_REPOSITORY https://gitlab.freedesktop.org/gstreamer/meson-ports/ffmpeg.git
    GIT_TAG meson-4.4

    # Workaround https://github.com/mesonbuild/meson/issues/11113
    # See https://gitlab.freedesktop.org/gstreamer/meson-ports/ffmpeg/-/issues/31
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-meson-Fix-library-detection-in-prefixed-builds.patch
          COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-ffmpeg-Add-an-option-to-disable-UB-in-av_uninit.patch
          COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-ffmpeg-Fix-crashes-in-ff_seek_frame_binary-if-built-with-la.patch
          COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0003-ffmpeg-Add-options-for-SSP-and-CFGuard.patch
          COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Disable-ebp_available-on-android.patch
          COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-libavutil-Don-t-disable-PIC-on-x86_32.patch
          COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-ffmpeg-Be-more-forceful-when-setting-PIC-on-x86_32.patch
          COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Copy-over-some-macros-from-libavutil-version.h.patch

    CONFIGURE_ARGS --prefix=${PREFIX_ext_ffmpeg}
            --libdir=lib
            -Dno_uninit_ub=true
            ${_stack_guard_flag}
            -Denable-cfguard=disabled # TODO: change to 'auto' when Krita enabled cfguard internally
            -Dbuildtype=$<IF:$<CONFIG:Debug>,debug,debugoptimized>
            -Ddefault_library=shared
            ${DISABLE_ASM_ON_ANDROID_x86_32}
            -Dcli=enabled
            -Dtests=disabled
            -Dffprobe=enabled
            -Dffplay=disabled
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
            -Dzlib=enabled
            -Dimage2_demuxer=enabled
            -Dimage2_alias_pix_demuxer=enabled
            -Dimage2_brender_pix_demuxer=enabled
            # ===================== Container Formats
            -Dwebm_muxer=enabled
            -Dmatroska_muxer=enabled
            -Dogg_muxer=enabled
            -Dapng_muxer=enabled
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

    UPDATE_COMMAND ""

    DEPENDS ${FLAC_DEP} ${OPUS_DEP} ${SDL_DEP} ${VPX_DEP} ext_libvorbis ext_libtheora ext_libopenh264 ext_libmp3lame ext_libaom ext_webp ext_png ext_jpeg ${MESON_DEP} ${PKG_CONFIG_DEP}
)
