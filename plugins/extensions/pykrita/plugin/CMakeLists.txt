# NOTE Disable trivial Qt keywords due conflicts w/ some Python.h header
# (at least version 3.3 of it has a member PyType_Spec::slots)
add_definitions(-DQT_NO_KEYWORDS)
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(SOURCES
    utilities.cpp
    PykritaModule.cpp
    PythonPluginManager.cpp
    PythonPluginsModel.cpp
)

ki18n_wrap_ui(SOURCES
    info.ui
    manager.ui
)

kis_add_library(pykrita STATIC ${SOURCES})

target_include_directories(pykrita SYSTEM PUBLIC "${PYTHON_INCLUDE_DIRS}")

target_link_libraries(
    pykrita
    ${PYTHON_LIBRARY}
    kritaui
    kritalibkis
  )

kis_add_library(kritapykrita MODULE
    plugin.cpp
    pyqtpluginsettings.cpp
  )

target_link_libraries(
    kritapykrita
    pykrita
  )

if (MINGW)
    target_compile_definitions(pykrita PRIVATE _hypot=hypot)
endif()
if (WIN32)
    # Banish Python 3.8 from mucking up Boost.Assert use of snprintf.
    # https://github.com/boostorg/system/issues/32
    target_compile_definitions(pykrita PRIVATE HAVE_SNPRINTF=1)
endif (WIN32)

install(TARGETS kritapykrita DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

# Install "built-in" api
install(
    DIRECTORY krita
    DESTINATION ${LIB_INSTALL_DIR}/krita-python-libs
    FILES_MATCHING PATTERN "*.py"
)

