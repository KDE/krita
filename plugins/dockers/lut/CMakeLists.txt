add_subdirectory(tests)

include_directories(SYSTEM
    ${OCIO_INCLUDE_DIR}
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

kis_add_library(kritalutdocker MODULE ${KRITA_LUTDOCKER_SOURCES})

target_link_libraries(kritalutdocker kritaui ${Boost_SYSTEM_LIBRARY} ${OCIO_LIBRARIES})
install(TARGETS kritalutdocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
