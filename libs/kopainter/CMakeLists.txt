include_directories( ${KOFFICECORE_INCLUDES} ${KOFFICEUI_INCLUDES} ${PIGMENT_INCLUDES} )


########### next target ###############

set(kopainter_LIB_SRCS
   svgpathparser.cc
   KoResourceChooser.cpp
   kogradientmanager.cc
   KoColorSlider.cc
   KoXYColorSelector.cc
   KoUniColorChooser.cpp
   KoUniColorDialog.cpp
   KoColorPatch.cpp
   KoDualColorButton.cpp
)

kde4_automoc(${kopainter_LIB_SRCS})

kde4_add_library(kopainter SHARED ${kopainter_LIB_SRCS})

target_link_libraries(kopainter  ${KDE4_KDECORE_LIBS} kofficeui pigment)

set_target_properties(kopainter PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kopainter  DESTINATION ${LIB_INSTALL_DIR})

