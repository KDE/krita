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

    CMAKE_ARGS ${LIBVORBIS_EXTRA_CMAKE_ARGS} -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_ffmpeg} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    DEPENDS ext_libogg
)

# ======================== VIDEO ===========================

ExternalProject_Add(
    ext_libopenh264
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    GIT_REPOSITORY git@github.com:cisco/openh264.git
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env
        CC="${CMAKE_C_COMPILER}"
        CXX="${CMAKE_CXX_COMPILER}"
        PYTHONPATH=${_krita_pythonpath}
        PKG_CONFIG=${PKG_CONFIG_EXECUTABLE}
        ${MESON_BINARY_PATH} <BINARY_DIR> <SOURCE_DIR>
        --prefix=${PREFIX_ext_ffmpeg}
    BUILD_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        ${MESON_BINARY_PATH} compile -C <BINARY_DIR> -j${SUBMAKE_JOBS}
    INSTALL_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        ${MESON_BINARY_PATH} install -C <BINARY_DIR>
    DEPENDS ext_nasm ${MESON_DEP} ${PKG_CONFIG_DEP}
)

if (WIN32)
    #TODO Proper windows build. See readme in lame.tar.gz sources.
    #ExternalProject_Add(
    #    ext_libmp3lame
    #    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    #    URL https://sourceforge.net/projects/lame/files/lame/3.100/lame-3.100.tar.gz
    #)
else()
    # *NIX
    ExternalProject_Add(
        ext_libmp3lame
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://sourceforge.net/projects/lame/files/lame/3.100/lame-3.100.tar.gz

        CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env <SOURCE_DIR>/configure --prefix=${PREFIX_ext_ffmpeg}

        BUILD_COMMAND ${CMAKE_COMMAND} -E env
            make
        
        INSTALL_COMMAND ${CMAKE_COMMAND} -E env
            make install
    )
endif()

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

    DEPENDS ${FLAC_DEP} ${VPX_DEP} ext_libvorbis ext_libopenh264 ext_libmp3lame ext_libaom ext_webp ext_png ext_jpeg ${MESON_DEP} ${PKG_CONFIG_DEP}
)
