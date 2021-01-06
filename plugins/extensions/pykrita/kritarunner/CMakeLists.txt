include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../plugin
                    ${CMAKE_CURRENT_BINARY_DIR}/../plugin
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libkis
                    ${CMAKE_CURRENT_BINARY_DIR}/../libkis
)

set(kritarunner_SRCS main.cpp)

ki18n_wrap_ui(kritarunner_SRCS
    ../plugin/info.ui
    ../plugin/manager.ui
)

add_executable(kritarunner ${kritarunner_SRCS})

target_include_directories(kritarunner SYSTEM PUBLIC "${PYTHON_INCLUDE_DIRS}")

target_link_libraries(kritarunner
                    PRIVATE
                      ${PYTHON_LIBRARY}
                      pykrita
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Widgets
                      Qt5::Xml
                      Qt5::Network
                      Qt5::PrintSupport
                      Qt5::Svg
                      Qt5::Concurrent)

if (MINGW)
    target_compile_definitions(kritarunner PRIVATE _hypot=hypot)
endif (MINGW)

install(TARGETS kritarunner ${INSTALL_TARGETS_DEFAULT_ARGS})
