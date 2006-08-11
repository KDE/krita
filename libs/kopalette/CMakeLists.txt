
include_directories( ${KOFFICECORE_INCLUDES} )


########### next target ###############

set(kopalette_LIB_SRCS 
   kopalette.cc 
   kopalettemanager.cc 
   kotoolboxpalette.cc 
   kotabpalette.cc
   KoPaletteTabBar.cpp
   KoPaletteTabWidget.cpp
   KoIconTabPalette.cpp)

kde4_automoc(${kopalette_LIB_SRCS})

kde4_add_library(kopalette SHARED ${kopalette_LIB_SRCS})

target_link_libraries(kopalette  ${KDE4_KDECORE_LIBS} kofficecore )

set_target_properties(kopalette PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kopalette  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

install( FILES  kopalettemanager.h   DESTINATION ${INCLUDE_INSTALL_DIR})

