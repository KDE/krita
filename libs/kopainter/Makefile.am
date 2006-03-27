INCLUDES= $(KOFFICECORE_INCLUDES) $(KOFFICEUI_INCLUDES) $(all_includes)

lib_LTLIBRARIES = libkopainter.la
libkopainter_la_SOURCES = svgpathparser.cc \
	koColor.cc \
	koFrameButton.cc \
	koColorSlider.cc \
	koColorChooser.cc \
	koIconChooser.cc \
	kogradientmanager.cc \
	ko_color_wheel.cc \
	ko_gray_widget.cc \
	ko_hsv_widget.cc \
	ko_rgb_widget.cc \
	ko_cmyk_widget.cc

libkopainter_la_LDFLAGS = $(all_libraries) -version-info 2:0:0 -no-undefined

libkopainter_la_LIBADD = $(LIB_KOFFICEUI)

METASOURCES = AUTO
