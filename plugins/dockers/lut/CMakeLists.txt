add_subdirectory(tests)

include_directories(SYSTEM
    ${OCIO_INCLUDE_DIR}
)

set(KRITA_LUTDOCKER_SOURCES
  lutdocker.cpp
  lutdocker_dock.cpp
  black_white_point_chooser.cpp
)

ki18n_wrap_ui(KRITA_LUTDOCKER_SOURCES
    wdglut.ui
)

if (HAVE_OCIO_V2)
  set(KRITA_LUTDOCKER_SOURCES
    ${KRITA_LUTDOCKER_SOURCES}
    ocio_display_filter_vfx2021.cpp
  )
  set(OCIO_ADDITIONAL_LIBRARIES "")
else()
  set(KRITA_LUTDOCKER_SOURCES
    ${KRITA_LUTDOCKER_SOURCES}
    ocio_display_filter_vfx2020.cpp
  )
  set(OCIO_ADDITIONAL_LIBRARIES ${Boost_SYSTEM_LIBRARY})
endif()

add_library(kritalutdocker MODULE ${KRITA_LUTDOCKER_SOURCES})

target_link_libraries(kritalutdocker kritaui ${OCIO_ADDITIONAL_LIBRARIES} OpenColorIO::OpenColorIO)
install(TARGETS kritalutdocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
