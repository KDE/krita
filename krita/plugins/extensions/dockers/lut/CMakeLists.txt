add_subdirectory(tests)
include_directories( ${OCIO_INCLUDE_DIR})
kde_enable_exceptions()

set(KRITA_LUTDOCKER_PART_SRCS
  lutdocker.cpp
  lutdocker_dock.cpp
  ocio_display_filter.cpp
  black_white_point_chooser.cpp
  )

ki18n_wrap_ui(KRITA_LUTDOCKER_PART_SRCS
    wdglut.ui
)

add_library(kritalutdocker MODULE ${KRITA_LUTDOCKER_PART_SRCS})
kcoreaddons_desktop_to_json(kritalutdocker krita_lutdocker.desktop)
generate_export_header(kritalutdocker BASE_NAME kritalutdocker)

target_link_libraries(kritalutdocker kritaui ${OCIO_LIBRARIES})

if(HAVE_OPENGL)
    target_link_libraries(kritalutdocker ${_LIBRARIES})
endif()

install(TARGETS kritalutdocker  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
