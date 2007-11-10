project(krita)

if (SHOULD_BUILD_KRITA)

    include(CheckFunctionExists)

    macro_optional_find_package(GLEW)
    macro_bool_to_01(GLEW_FOUND HAVE_GLEW)
    configure_file(config-glew.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-glew.h )

    if(HAVE_OPENGL)
        if(HAVE_GLEW)
            message(STATUS "GLEW found!")
        else(HAVE_GLEW)
            message(STATUS "GLEW NOT found - krita will not be able to use OpenGL shaders")
        endif(HAVE_GLEW)
    else(HAVE_OPENGL)
        message(STATUS "OpenGL NOT found - krita will not be able to use OpenGL for")
        message(STATUS "hardware accelerated rendering.")
    endif(HAVE_OPENGL)

    set(HAVE_XINPUTEXT 0)

    if(UNIX)
        set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES};m")
    endif(UNIX)

    check_function_exists(powf HAVE_POWF)

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-powf.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-powf.h)

    set(LINK_OPENEXR_LIB)
    if(OPENEXR_FOUND)
        include_directories(${OPENEXR_INCLUDE_DIR})
        set(LINK_OPENEXR_LIB ${OPENEXR_LIBRARIES})
        add_definitions(${OPENEXR_DEFINITIONS})
    endif(OPENEXR_FOUND)

    if(WIN32)
        if(MSVC)
            # C4522: 'class' : multiple assignment operators specified
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4522")
        endif(MSVC)
    endif(WIN32)

    set(KRITA_INCLUDES
        # For config-*.h
        ${CMAKE_CURRENT_BINARY_DIR}

        ${CMAKE_SOURCE_DIR}/krita/image
        ${CMAKE_SOURCE_DIR}/krita/image/tiles
        ${CMAKE_SOURCE_DIR}/krita/ui
        ${CMAKE_SOURCE_DIR}/krita/colorspaces/rgb_u8

        # For generated files from .ui
        ${CMAKE_BINARY_DIR}/krita/ui

        ${KOGUIUTILS_INCLUDES}
        ${LCMS_INCLUDE_DIR}
        ${PIGMENT_INCLUDES}
        ${KORESOURCES_INCLUDES}
        ${KOTEXT_INCLUDES}

        # 'Export' this for use by filters
        CACHE INTERNAL ""
        )

    include_directories(${KRITA_INCLUDES})

    if(HAVE_OPENGL)
        include_directories(${OPENGL_INCLUDE_DIR})
        if(HAVE_GLEW)
            include_directories(${GLEW_INCLUDE_DIR})
        endif(HAVE_GLEW)
    endif(HAVE_OPENGL)

    if(HAVE_XINPUTEXT)
        include_directories(${X11_Xinput_INCLUDE_PATH})
    endif(HAVE_XINPUTEXT)

    add_subdirectory( image )
    add_subdirectory( ui )
    add_subdirectory( shape )
    add_subdirectory( dtd )
    add_subdirectory( colorspaces )
    add_subdirectory( plugins )
    add_subdirectory( data )
    add_subdirectory( pics )


    ########### next target ###############

    set(kritapart_PART_SRCS krita_part_init.cc)

        kde4_add_plugin(kritapart WITH_PREFIX ${kritapart_PART_SRCS})

    

    target_link_libraries(kritapart kritaui)

install(TARGETS kritapart DESTINATION ${PLUGIN_INSTALL_DIR})



    ########### next target ###############

    set(krita_KDEINIT_SRCS main.cc)

    kde4_add_kdeinit_executable(krita ${krita_KDEINIT_SRCS})

    target_link_libraries(kdeinit_krita kritaui)

install(TARGETS kdeinit_krita DESTINATION ${LIB_INSTALL_DIR})

    target_link_libraries(krita ${QT_QTCORE_LIBRARY} kdeinit_krita)

install(TARGETS krita DESTINATION ${BIN_INSTALL_DIR})

    ########### install files ###############

install( FILES  krita.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES  kritapart.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  krita.rc krita_readonly.rc  DESTINATION ${DATA_INSTALL_DIR}/krita)

    #kde4_create_doxygen_docs( EXCLUDE CImg.h colorspaces plugins  image/tiles  )
endif (SHOULD_BUILD_KRITA)
