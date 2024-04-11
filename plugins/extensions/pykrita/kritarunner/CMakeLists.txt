include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../plugin
                    ${CMAKE_CURRENT_BINARY_DIR}/../plugin
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libkis
                    ${CMAKE_CURRENT_BINARY_DIR}/../libkis
)

set(kritarunner_SRCS main.cpp)

set(kritarunner_PRIVATE_LINK_LIBS
    ${PYTHON_LIBRARY}
    pykrita
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Xml
    Qt6::Network
    Qt6::PrintSupport
    Qt6::Svg
    Qt6::Concurrent
)

if(WIN32)
    configure_file(kritarunner.exe.manifest.in ${CMAKE_CURRENT_BINARY_DIR}/kritarunner.exe.manifest)
    if (MINGW)
        # CMake does not know how to embed manifest on mingw, so we create our own RC file to be included
        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/kritarunner.exe.manifest.rc "1 24 \"kritarunner.exe.manifest\"\n")
        add_library(kritarunner_manifest_res OBJECT ${CMAKE_CURRENT_BINARY_DIR}/kritarunner.exe.manifest.rc)
        set_property(TARGET kritarunner_manifest_res PROPERTY AUTOMOC OFF)
        set_property(TARGET kritarunner_manifest_res PROPERTY INCLUDE_DIRECTORIES "")
        set(kritarunner_SRCS ${kritarunner_SRCS} $<TARGET_OBJECTS:kritarunner_manifest_res>)
    else()
        set(kritarunner_SRCS ${kritarunner_SRCS} 
            ${CMAKE_CURRENT_BINARY_DIR}/kritarunner.exe.manifest
        )
    endif()
endif()

add_executable(kritarunner ${kritarunner_SRCS})
target_include_directories(kritarunner SYSTEM PUBLIC "${PYTHON_INCLUDE_DIRS}")
target_link_libraries(kritarunner PRIVATE ${kritarunner_PRIVATE_LINK_LIBS})

if(APPLE)
    set_target_properties(kritarunner PROPERTIES INSTALL_RPATH "@loader_path/../Frameworks;@executable_path/../lib;@executable_path/../Frameworks")
    set_property(TARGET kritarunner PROPERTY MACOSX_BUNDLE OFF)
endif()

if (MINGW)
    target_compile_definitions(kritarunner PRIVATE _hypot=hypot)
endif (MINGW)

install(TARGETS kritarunner ${INSTALL_TARGETS_DEFAULT_ARGS})

if (WIN32)
    add_executable(kritarunner_com ${kritarunner_SRCS})
    target_include_directories(kritarunner_com SYSTEM PUBLIC "${PYTHON_INCLUDE_DIRS}")
    target_link_libraries(kritarunner_com PRIVATE ${kritarunner_PRIVATE_LINK_LIBS})

    set_target_properties(kritarunner_com
        PROPERTIES
            WIN32_EXECUTABLE false
            SUFFIX ".com"
    )
    if (NOT MSVC)
        # MSBuild copies all binaries and library exports into a single 
        # directory. This constitutes a write race condition with kritarunner.
        # See https://gitlab.kitware.com/cmake/cmake/-/issues/22133
        set_target_properties(kritarunner_com
            PROPERTIES
                OUTPUT_NAME "kritarunner"
        )
    endif()

    if (MINGW)
        target_compile_definitions(kritarunner_com PRIVATE _hypot=hypot)
    endif (MINGW)

    if (MSVC)
        target_link_options(kritarunner_com PRIVATE "/DEBUG:NONE")
    endif()

    install(TARGETS kritarunner_com ${INSTALL_TARGETS_DEFAULT_ARGS})
endif (WIN32)
