# NOTE Disable trivial Qt keywords due conflicts w/ some Python.h header
# (at least version 3.3 of it has a member PyType_Spec::slots)
add_definitions(-DQT_NO_KEYWORDS)
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(SOURCES
    plugin.cpp
    pyqtpluginsettings.cpp
    utilities.cpp
    PykritaModule.cpp
    PythonPluginManager.cpp
    PythonPluginsModel.cpp
)

ki18n_wrap_ui(SOURCES
    info.ui
    manager.ui
)

add_library(kritapykrita MODULE ${SOURCES})
add_library(pykrita STATIC ${SOURCES})

target_include_directories(kritapykrita SYSTEM PUBLIC "${PYTHON_INCLUDE_DIRS}")
target_include_directories(pykrita SYSTEM PUBLIC "${PYTHON_INCLUDE_DIRS}")

target_link_libraries(
    kritapykrita
    ${PYTHON_LIBRARY}
    kritaui
    kritalibkis
  )

target_link_libraries(
    pykrita
    ${PYTHON_LIBRARY}
    kritaui
    kritalibkis
  )

if (MINGW)
    target_compile_definitions(kritapykrita PRIVATE _hypot=hypot)
    target_compile_definitions(pykrita PRIVATE _hypot=hypot)
endif (MINGW)

install(TARGETS kritapykrita DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

# Install "built-in" api
install(
    DIRECTORY krita
    DESTINATION ${LIB_INSTALL_DIR}/krita-python-libs
    FILES_MATCHING PATTERN "*.py"
)

