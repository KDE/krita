include_directories(SYSTEM
                    ${GSTREAMER_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS})

set(KRITA_RECORDERDOCKER_SOURCES recorderdocker.cpp recorderdocker_dock.cpp encoder.cpp)
add_library(kritarecorderdocker MODULE ${KRITA_RECORDERDOCKER_SOURCES})
target_link_libraries(kritarecorderdocker kritaui ${GSTREAMER_LIBRARIES} ${GSTREAMER_BASE_LIBRARIES} ${GLIB_LIBRARIES} ${GOBJECT_LIBRARIES} ${GSTREAMER_APP_LIBRARIES})

install(TARGETS kritarecorderdocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
