

include_directories( ${KOFFICECORE_INCLUDES} )


########### next target ###############

set(kopainter_LIB_SRCS 
   svgpathparser.cc 
   koColor.cc 
   koFrameButton.cc 
   koIconChooser.cc 
   kogradientmanager.cc 
   ko_color_wheel.cc
   KoColorSlider.cc 
   KoXYColorSelector.cc 
   ko_gray_widget.cc 
   ko_hsv_widget.cc 
   KoUniColorChooser.cpp 
   ko_rgb_widget.cc )

kde4_automoc(${kopainter_LIB_SRCS})

kde4_add_library(kopainter SHARED ${kopainter_LIB_SRCS})

target_link_libraries(kopainter  ${KDE4_KDECORE_LIBS} kofficeui pigment)

set_target_properties(kopainter PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install_targets(${LIB_INSTALL_DIR} kopainter )


########### install files ###############






#original Makefile.am contents follow:

#INCLUDES= $(KOFFICECORE_INCLUDES) $(KOFFICEUI_INCLUDES) $(all_includes)
#
#lib_LTLIBRARIES = libkopainter.la
#libkopainter_la_SOURCES = svgpathparser.cc \
#	koColor.cc \
#	koFrameButton.cc \
#	koColorSlider.cc \
#	koColorChooser.cc \
#	koIconChooser.cc \
#	kogradientmanager.cc \
#	ko_color_wheel.cc \
#	ko_gray_widget.cc \
#	ko_hsv_widget.cc \
#	ko_rgb_widget.cc \
#	ko_cmyk_widget.cc
#
#libkopainter_la_LDFLAGS = $(all_libraries) -version-info 2:0:0 -no-undefined
#
#libkopainter_la_LIBADD = $(LIB_KOFFICEUI)
#
#METASOURCES = AUTO
