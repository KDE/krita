include_directories(SYSTEM
                    ${VPX_INCLUDE_DIR})

set(KRITA_RECORDERDOCKER_SOURCES recorderdocker.cpp recorderdocker_dock.cpp encoder.cpp ivfenc.cpp)
add_library(kritarecorderdocker MODULE ${KRITA_RECORDERDOCKER_SOURCES})
target_link_libraries(kritarecorderdocker kritaui ${VPX_LIBRARIES} ${YUV_LIBRARIES} ${JPEG_LIBRARIES})

install(TARGETS kritarecorderdocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
