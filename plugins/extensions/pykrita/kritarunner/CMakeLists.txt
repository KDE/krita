include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../plugin
                    ${CMAKE_CURRENT_BINARY_DIR}/../plugin
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libkis
                    ${CMAKE_CURRENT_BINARY_DIR}/../libkis
)

set(kritarunner_SRCS main.cpp
    ../plugin/plugin.cpp
    ../plugin/pyqtpluginsettings.cpp
    ../plugin/utilities.cpp
    ../plugin/PykritaModule.cpp
    ../plugin/PythonPluginManager.cpp
    ../plugin/PythonPluginsModel.cpp
)

add_executable(kritarunner ${kritarunner_SRCS})
target_link_libraries(kritarunner 
                    PRIVATE
                      ${PYTHON_LIBRARY}
                      kritaui
                      kritalibkis
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
