

add_subdirectory( colorspaces )
if(KUNITTEST_FOUND)
    add_subdirectory(tests)
endif(KUNITTEST_FOUND)




########### next target ###############
set(kritacolor_LIB_SRCS 
   kis_histogram_producer.cc 
   kis_basic_histogram_producers.cc 
   kis_color_conversions.cc 
   )
#   ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/kis_xyz_colorspace.cc
#   ${CMAKE_CURRENT_SOURCE_DIR}/colorspaces/kis_xyz_colorspace.h

kde4_automoc(${kritacolor_LIB_SRCS})

kde4_add_library(kritacolor SHARED ${kritacolor_LIB_SRCS})

target_link_libraries(kritacolor ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} ${LCMS_LIBRARIES} ${LINK_OPENEXR_LIB} pigment kofficecore)

set_target_properties(kritacolor PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kritacolor  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

install( FILES  kis_histogram_producer.h      kis_basic_histogram_producers.h  DESTINATION ${INCLUDE_INSTALL_DIR})




