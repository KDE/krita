if (NOT WIN32 AND NOT APPLE)
    add_subdirectory(tests)
endif()

include_directories( ${OCIO_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
)

set(KRITA_LUTDOCKER_SOURCES
  lutdocker.cpp
  lutdocker_dock.cpp
  ocio_display_filter.cpp
  black_white_point_chooser.cpp
)

ki18n_wrap_ui(KRITA_LUTDOCKER_SOURCES
    wdglut.ui
)

add_library(kritalutdocker MODULE ${KRITA_LUTDOCKER_SOURCES})

generate_export_header(kritalutdocker BASE_NAME kritalutdocker)
target_link_libraries(kritalutdocker kritaui ${Boost_SYSTEM_LIBRARY} ${OCIO_LIBRARIES})
install(TARGETS kritalutdocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
