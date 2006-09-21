project(krita)

macro_optional_find_package(Kunittest)

set(REQUIRED_LCMS_VERSION 112)

if(LCMS_FOUND AND NOT LCMS_VERSION LESS ${REQUIRED_LCMS_VERSION})
    set(SHOULD_BUILD_KRITA TRUE)
    if(LCMS_VERSION EQUAL 114)
        message(STATUS "==============================================")
        message(STATUS "WARNING: you are using version 1.14 of lcms")
        message(STATUS " this version has a bug that break filling")
        message(STATUS " and select similar colors in at least CMYK")
        message(STATUS " you might want to use a newer version, or")
        message(STATUS " an older")
        message(STATUS "==============================================")
    endif(LCMS_VERSION EQUAL 114)
else(LCMS_FOUND AND NOT LCMS_VERSION LESS ${REQUIRED_LCMS_VERSION})
	set(SHOULD_BUILD_KRITA FALSE)
	message(STATUS  "krita requires LittleCMS 1.12 or greater. Krita will not be built.\n"
			"   If you want to compile Krita you should install:\n"
			"   lcms 1.12 or newer (http://www.littlecms.com/)\n"
			"   and avoid 1.14 as it has a major bug in CMYK")
endif(LCMS_FOUND AND NOT LCMS_VERSION LESS ${REQUIRED_LCMS_VERSION})

if (SHOULD_BUILD_KRITA)

    macro_optional_find_package(OpenGL)

    set(HAVE_OPENGL 0)

    if(OPENGL_FOUND)
        message(STATUS "Found OpenGL: ${OPENGL_LIBRARIES}")
        if(QT_QTOPENGL_FOUND)
            message(STATUS "Found Qt OpenGL support")
            set(HAVE_OPENGL 1)
        else(QT_QTOPENGL_FOUND)
            message(STATUS "Did NOT find Qt OpenGL support. Check your Qt configuration")
        endif(QT_QTOPENGL_FOUND)
    else(OPENGL_FOUND)
        message(STATUS "Did NOT find OpenGL libraries")
    endif(OPENGL_FOUND)

    if(NOT HAVE_OPENGL)
        message(STATUS " * krita will not be able to use OpenGL for")
        message(STATUS "   hardware accelerated rendering.")
    endif(NOT HAVE_OPENGL)

    if(X11_Xinput_FOUND)
        set(HAVE_XINPUTEXT 1)
        message(STATUS "Found XInput extension")
    else(X11_Xinput_FOUND)
        set(HAVE_XINPUTEXT 0)
    endif(X11_Xinput_FOUND)

    if(UNIX)
        set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES};m")
    endif(UNIX)

    check_function_exists(powf HAVE_POWF)

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-krita.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-krita.h)

	set(LINK_OPENEXR_LIB)
	if(OPENEXR_FOUND)
    	include_directories(${OPENEXR_INCLUDE_DIR})
    	set(LINK_OPENEXR_LIB ${OPENEXR_LIBRARIES})
	endif(OPENEXR_FOUND)


    set(KRITA_INCLUDES
        # For config-krita.h
        ${CMAKE_CURRENT_BINARY_DIR}

        ${CMAKE_SOURCE_DIR}/krita/image
        ${CMAKE_SOURCE_DIR}/krita/image/tiles
        ${CMAKE_SOURCE_DIR}/krita/kritacolor
        ${CMAKE_SOURCE_DIR}/krita/kritacolor/colorspaces
        ${CMAKE_SOURCE_DIR}/krita/ui
        ${CMAKE_SOURCE_DIR}/krita/colorspaces/rgb_u8

        # For generated files from .ui
        ${CMAKE_BINARY_DIR}/krita/ui

        ${KOPAINTER_INCLUDES}
        ${KOPALETTE_INCLUDES}
        ${KOFFICEUI_INCLUDES}
        ${LCMS_INCLUDE_DIR}
        ${PIGMENT_INCLUDES}

        # 'Export' this for use by filters
        CACHE INTERNAL ""
        )

    include_directories(${KRITA_INCLUDES})

    if(HAVE_OPENGL)
        include_directories(${OPENGL_INCLUDE_DIR})
    endif(HAVE_OPENGL)

    if(HAVE_XINPUTEXT)
        include_directories(${X11_Xinput_INCLUDE_PATH})
    endif(HAVE_XINPUTEXT)

    add_subdirectory( kritacolor )
    add_subdirectory( image )
    add_subdirectory( ui )
    add_subdirectory( dtd )
    add_subdirectory( colorspaces )
    add_subdirectory( plugins )
    add_subdirectory( data )
    add_subdirectory( pics )


    ########### next target ###############

    set(kritapart_PART_SRCS krita_part_init.cc)

    kde4_automoc(${kritapart_PART_SRCS})

    kde4_add_plugin(kritapart WITH_PREFIX ${kritapart_PART_SRCS})

    kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritapart )

    target_link_libraries(kritapart kritaui)

install(TARGETS kritapart DESTINATION ${PLUGIN_INSTALL_DIR})



    ########### next target ###############

    set(krita_KDEINIT_SRCS main.cc)

    kde4_automoc(${krita_KDEINIT_SRCS})

    kde4_add_kdeinit_executable(krita ${krita_KDEINIT_SRCS})

    target_link_libraries(kdeinit_krita kritaui)

install(TARGETS kdeinit_krita DESTINATION ${LIB_INSTALL_DIR})

    target_link_libraries(krita kdeinit_krita)

install(TARGETS krita DESTINATION ${BIN_INSTALL_DIR})

    ########### install files ###############

install( FILES  krita.desktop  DESTINATION ${XDG_APPS_DIR})
install( FILES  kritapart.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  krita.rc krita_readonly.rc  DESTINATION ${DATA_INSTALL_DIR}/krita)

    #kde4_create_doxygen_docs( EXCLUDE CImg.h colorspaces plugins kritacolor/colorspaces image/tiles  )
endif (SHOULD_BUILD_KRITA)
