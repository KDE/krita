include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../plugin
                    ${CMAKE_CURRENT_BINARY_DIR}/../plugin
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libkis
                    ${CMAKE_CURRENT_BINARY_DIR}/../libkis
)

set(kritarunner_SRCS main.cpp)

set(kritarunner_PRIVATE_LINK_LIBS
    ${PYTHON_LIBRARY}
    pykrita
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Xml
    Qt5::Network
    Qt5::PrintSupport
    Qt5::Svg
    Qt5::Concurrent
)

add_executable(kritarunner ${kritarunner_SRCS})
target_include_directories(kritarunner SYSTEM PUBLIC "${PYTHON_INCLUDE_DIRS}")
target_link_libraries(kritarunner PRIVATE ${kritarunner_PRIVATE_LINK_LIBS})

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
            OUTPUT_NAME "kritarunner"
            SUFFIX ".com"
    )

    if (MINGW)
        target_compile_definitions(kritarunner_com PRIVATE _hypot=hypot)
    endif (MINGW)

    if (MSVC)
        target_link_options(kritarunner_com PRIVATE "/DEBUG:NONE")
    endif()

    install(TARGETS kritarunner_com ${INSTALL_TARGETS_DEFAULT_ARGS})
endif (WIN32)
