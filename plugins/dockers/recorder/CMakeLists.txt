include_directories(/usr/local/include/gstreamer-1.0
                    /usr/include/glib-2.0
                    /usr/lib/x86_64-linux-gnu/glib-2.0/include)

set(KRITA_RECORDERDOCKER_SOURCES recorderdocker.cpp recorderdocker_dock.cpp encoder.cpp)
add_library(kritarecorderdocker MODULE ${KRITA_RECORDERDOCKER_SOURCES})
target_link_libraries(kritarecorderdocker kritaui)
target_link_libraries(kritarecorderdocker gstreamer-1.0)
target_link_libraries(kritarecorderdocker glib-2.0)
target_link_libraries(kritarecorderdocker gobject-2.0)
target_link_libraries(kritarecorderdocker gstapp-1.0)
target_link_libraries(kritarecorderdocker gstbase-1.0)
install(TARGETS kritarecorderdocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
