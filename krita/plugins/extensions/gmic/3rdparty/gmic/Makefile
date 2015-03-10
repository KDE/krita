#
#  File        : Makefile
#                ( Makefile for GNU 'make' utility )
#
#  Description : Makefile for compiling :
#
#                 . the G'MIC command line tool (type 'make').
#                 . the G'MIC plug-in for GIMP (type 'make gimp').
#                 . the G'MIC interpreter, as a C++ library (type 'make lib').
#                 . the ZArt software (type 'make zart').
#
#                ( http://gmic.eu )
#
#  Copyright   : David Tschumperle
#                ( http://tschumperle.users.greyc.fr/ )
#
#  License     : CeCILL v2.0
#                ( http://www.cecill.info/licences/Licence_CeCILL_V2-en.html )
#
#  This software is governed by the CeCILL  license under French law and
#  abiding by the rules of distribution of free software.  You can  use,
#  modify and/ or redistribute the software under the terms of the CeCILL
#  license as circulated by CEA, CNRS and INRIA at the following URL
#  "http://www.cecill.info".
#
#  As a counterpart to the access to the source code and  rights to copy,
#  modify and redistribute granted by the license, users are provided only
#  with a limited warranty  and the software's author,  the holder of the
#  economic rights,  and the successive licensors  have only  limited
#  liability.
#
#  In this respect, the user's attention is drawn to the risks associated
#  with loading,  using,  modifying and/or developing or reproducing the
#  software by the user in light of its specific status of free software,
#  that may mean  that it is complicated to manipulate,  and  that  also
#  therefore means  that it is reserved for developers  and  experienced
#  professionals having in-depth computer knowledge. Users are therefore
#  encouraged to load and test the software's suitability as regards their
#  requirements in conditions enabling the security of their systems and/or
#  data to be ensured and,  more generally, to use and operate it in the
#  same conditions as regards security.
#
#  The fact that you are presently reading this means that you have had
#  knowledge of the CeCILL license and that you accept its terms.
#

#----------------------------------
# Set correct variables and paths.
#----------------------------------
OS := $(shell uname)
CC = g++
ifeq ($(OS),Linux)
OS = Unix
endif
ifeq ($(OS),GNU/kFreeBSD)
OS = Unix
endif
ifeq ($(OS),GNU)
OS = Unix
endif
ifeq ($(OS),Darwin)
PLUGINDIR = ~/Library/Application\ Support/Gimp/plug-ins
USR = /opt/local
else
PLUGINDIR = `gimptool-2.0 --gimpplugindir`/plug-ins
USR = /usr
endif
ifeq ($(OSTYPE),msys)
EXE = .exe
endif
LIB=lib
BIN=bin
INCLUDE=include

#------------------------------------------------
# Set compilation flags for build customization.
#------------------------------------------------

# Flags to enable 'beta' version.
IS_BETA_CFLAGS = -Dgmic_is_beta=1

# Flags that are mandatory to compile 'gmic'.
ifndef NO_STDLIB
MANDATORY_CFLAGS += -std=c++11 -Dgmic_build -I$(USR)/$(INCLUDE)
else
MANDATORY_CFLAGS += -Dgmic_build -I$(USR)/$(INCLUDE)
endif
MANDATORY_LIBS += -L$(USR)/$(LIB)
ifeq ($(notdir $(CC)),g++)
MANDATORY_CFLAGS += -Wall -W
MANDATORY_LIBS += -lm
endif
MANDATORY_CFLAGS += $(IS_BETA_CFLAGS)
ifeq ($(OS),Unix)
MANDATORY_CFLAGS += -Dcimg_use_vt100
endif
ifeq ($(OS),Darwin)
ifndef NO_STDLIB
MANDATORY_CFLAGS += -stdlib=libc++
endif
endif
ifeq ($(OSTYPE),msys)
MANDATORY_LIBS = -Wl,--stack,16777216
endif

# Flags to enable debugging.
DEBUG_CFLAGS = -ansi -pedantic -Dcimg_verbosity=3 -g

# Flags to enable optimizations.
ifeq ($(notdir $(CC)),g++)
ifeq ($(OS),Darwin)
OPT_CFLAGS = -O2 -mtune=generic
else
OPT_CFLAGS = -O2 -fno-ipa-sra -mtune=generic
endif
else
ifeq ($(notdir $(CC)),icpc)
OPT_CFLAGS = -fast
else
OPT_CFLAGS = -O2
endif
endif

# Flags to enable parallel processing.
ifneq ($(OS),Darwin)
PARALLEL_CFLAGS = -Dgmic_is_parallel
ifneq ($(OSTYPE),msys)
PARALLEL_LIBS = -lpthread
endif
endif

# Flags to enable extra checking of image buffer pointers.
ifneq ($(OS),Darwin)
CHECKIMAGE_CFLAGS = -Dgmic_check_image
endif

# Flags to enable parallelization using OpenMP.
OPENMP_CFLAGS = -fopenmp -Dcimg_use_openmp
OPENMP_LIBS = -lgomp

# Flags to enable image display, using X11
# (keep /usr/ dirname here since X11 is located in /usr/ on Mac too).
# This requires the presence of the X11 include and library files.
# (package 'libx11-dev' on Debian).
X11_CFLAGS = -Dcimg_display=1 -Dcimg_appname=\\\"gmic\\\" -I/usr/X11R6/include #-Dcimg_use_xrandr
X11_LIBS = -L/usr/X11R6/lib -lX11 -lpthread #-lXrandr

# Flags to enable fast display, using XShm.
# This requires the presence of the X11 extension include and library files.
# (package 'libx11-dev' on Debian).
XSHM_CFLAGS = -Dcimg_use_xshm
XSHM_LIBS = -L$(USR)/X11R6/lib -lXext

# Flags to enable image display, using GDI32.
# This requires the presence of the GDI32 include and library files.
GDI32_CFLAGS = -Dcimg_display=2 -Dcimg_appname=\\\"gmic\\\"
GDI32_LIBS = -lgdi32

# Flags to enable native support for PNG image files, using the PNG library.
# This requires the presence of the libpng include and library files.
# (package 'libpng12-dev' on Debian).
PNG_CFLAGS = -Dcimg_use_png
PNG_LIBS = -lpng -lz

# Flags to enable native support for JPEG image files, using the JPEG library.
# This requires the presence of the libjpeg include and library files.
# (package 'libjpeg62-dev' on Debian).
JPEG_CFLAGS = -Dcimg_use_jpeg
JPEG_LIBS = -ljpeg

# Flags to enable native support for TIFF image files, using the TIFF library.
# This requires the presence of the libtiff include and library files.
# (package 'libtiff4-dev' on Debian).
TIFF_CFLAGS = -Dcimg_use_tiff
TIFF_LIBS = -ltiff

# Flags to enable native support for MINC2 image files, using the MINC2 library.
# ( http://en.wikibooks.org/wiki/MINC/Reference/MINC2.0_Users_Guide )
# (package 'libminc-dev' on Debian).
MINC2_CFLAGS = -Dcimg_use_minc2 -I${HOME}/local/include
MINC2_LIBS = -lminc_io -lvolume_io2 -lminc2 -lnetcdf -lhdf5 -lz -L${HOME}/local/lib

# Flags to enable native support for compressed .cimgz files, using the Zlib library.
# This requires the presence of the Zlib include and library files.
# (package 'zlib1g-dev' on Debian).
ZLIB_CFLAGS = -Dcimg_use_zlib
ZLIB_LIBS = -lz

# Flags to enable native support for downloading files from the network.
# ( http://curl.haxx.se/libcurl/ )
CURL_CFLAGS = -Dcimg_use_curl
CURL_LIBS = -lcurl

# Flags to enable native support of webcams, using the OpenCV library.
# This requires the presence of the OpenCV include and library files.
# (package 'libcv3-2-dev' on Debian).
ifeq ($(OS),Darwin)
OPENCV_CFLAGS = -Dcimg_use_opencv -I$(USR)/$(INCLUDE) -I$(USR)/$(INCLUDE)/opencv
OPENCV_LIBS = `pkg-config opencv --libs`   #-> Use this for OpenCV 2.2.0 !
else
OPENCV_CFLAGS = -Dcimg_use_opencv -I$(USR)/$(INCLUDE) -I$(USR)/$(INCLUDE)/opencv
# OPENCV_LIBS = -lcv -lhighgui
OPENCV_LIBS = -lopencv_core -lopencv_highgui   #-> Use this for OpenCV >= 2.2.0 !
endif

# Flags to enable native support of most classical image file formats, using the GraphicsMagick++ library.
# This requires the presence of the GraphicsMagick++ include and library files.
# (package 'libgraphicsmagick++1-dev' on Debian).
MAGICK_CFLAGS = -Dcimg_use_magick -I$(USR)/$(INCLUDE)/GraphicsMagick
ifeq ($(OS),Darwin)
MAGICK_LIBS = -L$(USR)/$(LIB) -lGraphicsMagick++ -lGraphicsMagick -llcms -ltiff -lfreetype -ljpeg -lpng -lbz2 -lxml2 -lz -lm -lltdl
else
MAGICK_LIBS = -lGraphicsMagick++
endif

# Flags to enable native support of EXR file format, using the OpenEXR library/
# This requires the presence of the OpenEXR include and library files.
# (package 'libopenexr-dev' on Debian).
EXR_CFLAGS = -Dcimg_use_openexr -I$(USR)/$(INCLUDE)/OpenEXR
EXR_LIBS = -lIlmImf -lHalf

# Flags to enable the use of the FFTW3 library.
# This requires the presence of the FFTW3 include and library files.
# (package 'libfftw3-dev' on Debian).
FFTW_CFLAGS = -Dcimg_use_fftw3
ifeq ($(OSTYPE),msys)
FFTW_LIBS = -lfftw3
else
FFTW_LIBS = -lfftw3 -lfftw3_threads
endif

# Flags to enable the use of the BOARD library.
# This requires the presence of the BOARD include and library files.
# (no packages exist for Debian at this time).
BOARD_CFLAGS = -Dcimg_use_board
BOARD_LIBS = -lboard

#----------------------------------------------------------------
# Predefined sets of flags for different default configurations.
#----------------------------------------------------------------

# Unix : Standard build.
STD_UNIX_CFLAGS = $(MANDATORY_CFLAGS) $(PARALLEL_CFLAGS) $(OPENMP_CFLAGS) $(X11_CFLAGS) $(PNG_CFLAGS) \
                  $(JPEG_CFLAGS) $(TIFF_CFLAGS) $(ZLIB_CFLAGS) $(CURL_CFLAGS) \
	  	  $(EXR_CFLAGS) $(FFTW_CFLAGS) $(OPENCV_CFLAGS) # $(XSHM_CFLAGS) $(MAGICK_CFLAGS)
STD_UNIX_LIBS = $(MANDATORY_LIBS) $(PARALLEL_LIBS) $(OPENMP_LIBS) $(X11_LIBS) $(PNG_LIBS) \
		   $(JPEG_LIBS) $(TIFF_LIBS) $(ZLIB_LIBS) $(CURL_LIBS) \
	           $(EXR_LIBS) $(FFTW_LIBS) $(OPENCV_LIBS) # $(XSHM_LIBS) # $(MAGICK_LIBS)

# Unix : Minimal build.
MINIMAL_UNIX_CFLAGS = $(MANDATORY_CFLAGS) $(PARALLEL_CFLAGS) $(OPENMP_CFLAGS) $(DEBUG_CFLAGS) $(X11_CFLAGS) \
	              ${TIFF_CFLAGS} ${PNG_CFLAGS} $(ZLIB_CFLAGS) $(CURL_CFLAGS) \
		      $(FFTW_CFLAGS) $(CHECKIMAGE_CFLAGS) $(OPENCV_CFLAGS) \

MINIMAL_UNIX_LIBS = $(MANDATORY_LIBS) $(PARALLEL_LIBS) $(OPENMP_LIBS) $(X11_LIBS) \
	            ${TIFF_LIBS} ${PNG_LIBS} $(ZLIB_LIBS) $(CURL_LIBS) $(FFTW_LIBS) $(OPENCV_LIBS) \

# Unix : Static build.
STATIC_PATH = $(USR)/${LIB}/x86_64-linux-gnu
STATIC_EXTRA =
STATIC_UNIX_CFLAGS = $(MANDATORY_CFLAGS) $(PARALLEL_CFLAGS) $(PNG_CFLAGS) $(JPEG_CFLAGS) $(ZLIB_CFLAGS) $(FFTW_CFLAGS) -Dcimg_display=0
STATIC_UNIX_LIBS = $(PARALLEL_LIBS) \
	              $(STATIC_PATH)/libpng.a \
	              $(STATIC_PATH)/libjpeg.a \
                      $(STATIC_PATH)/libz.a \
                      $(STATIC_PATH)/libfftw3.a $(STATIC_PATH)/libfftw3_threads.a $(STATIC_EXTRA)

# Unix : Custom build.
CUSTOM_UNIX_CFLAGS = $(MANDATORY_CFLAGS) $(PARALLEL_CFLAGS) $(OPENMP_CFLAGS) $(X11_CFLAGS) $(PNG_CFLAGS) \
                     $(JPEG_CFLAGS) $(TIFF_CFLAGS) $(ZLIB_CFLAGS) $(CURL_CFLAGS) \
	    	     $(EXR_CFLAGS) $(FFTW_CFLAGS) $(OPENCV_CFLAGS)
CUSTOM_UNIX_LIBS = $(MANDATORY_LIBS) $(PARALLEL_LIBS) $(OPENMP_LIBS) $(X11_LIBS) $(PNG_LIBS) \
		   $(JPEG_LIBS) $(TIFF_LIBS) $(ZLIB_LIBS) $(CURL_LIBS) \
	           $(EXR_LIBS) $(FFTW_LIBS) $(OPENCV_LIBS)

# Windows : Standard build.
STD_WINDOWS_CFLAGS= $(MANDATORY_CFLAGS) $(PARALLEL_CFLAGS) $(OPENMP_CFLAGS) $(GDI32_CFLAGS) $(PNG_CFLAGS) $(JPEG_CFLAGS) $(TIFF_CFLAGS) $(ZLIB_CFLAGS) $(CURL_CFLAGS) $(FFTW_CFLAGS) $(OPENCV_CFLAGS)
STD_WINDOWS_LIBS = $(MANDATORY_LIBS) $(PARALLEL_LIBS) $(OPENMP_LIBS) $(GDI32_LIBS) $(PNG_LIBS) $(JPEG_LIBS) $(TIFF_LIBS) $(ZLIB_LIBS) $(CURL_LIBS) $(FFTW_LIBS) $(OPENCV_LIBS)

# Windows : Minimal build.
MINIMAL_WINDOWS_CFLAGS = $(MANDATORY_CFLAGS) $(PARALLEL_CFLAGS) $(OPENMP_CFLAGS) $(DEBUG_CFLAGS) $(GDI32_CFLAGS) $(CHECKIMAGE_CFLAGS)
MINIMAL_WINDOWS_LIBS = $(MANDATORY_LIBS) $(PARALLEL_LIBS) $(OPENMP_LIBS) $(GDI32_LIBS)

# Mac-OSX : Standard build.
STD_MACOSX_CFLAGS= $(MANDATORY_CFLAGS) $(X11_CFLAGS) $(ZLIB_CFLAGS) $(PNG_CFLAGS) \
                   $(JPEG_CFLAGS) $(TIFF_CFLAGS) \
		   $(EXR_CFLAGS) $(FFTW_CFLAGS) # $(MAGICK_CFLAGS)
STD_MACOSX_LIBS = $(MANDATORY_LIBS) $(X11_LIBS) $(ZLIB_LIBS) \
		     $(PNG_LIBS) $(JPEG_LIBS) $(TIFF_LIBS) \
		     $(EXR_LIBS) $(FFTW_LIBS) # $(MAGICK_LIBS)

# Mac-OSX : Minimal build.
MINIMAL_MACOSX_CFLAGS = $(MANDATORY_CFLAGS) $(DEBUG_CFLAGS) $(X11_CFLAGS) \
	              ${TIFF_CFLAGS} ${PNG_CFLAGS} $(ZLIB_CFLAGS) $(FFTW_CFLAGS) $(CHECKIMAGE_CFLAGS) $(OPENCV_CFLAGS)
MINIMAL_MACOSX_LIBS = $(MANDATORY_LIBS) $(X11_LIBS) \
	               ${TIFF_LIBS} ${PNG_LIBS} $(ZLIB_LIBS) $(FFTW_LIBS) $(OPENCV_LIBS)

# GIMP plug-in : Standard build.
STD_GIMP_CFLAGS = $(MANDATORY_CFLAGS) $(FFTW_CFLAGS) $(PNG_CFLAGS) $(ZLIB_CFLAGS) -Dcimg_use_rng
STD_GIMP_LIBS = $(MANDATORY_LIBS) $(FFTW_LIBS) $(PNG_LIBS) $(ZLIB_LIBS)

ifeq ($(OS),Unix)
STD_GIMP_CFLAGS += $(PARALLEL_CFLAGS) $(OPENMP_CFLAGS) $(X11_CFLAGS) $(CURL_CFLAGS)
STD_GIMP_LIBS += $(PARALLEL_LIBS) $(OPENMP_LIBS) $(X11_LIBS) $(CURL_LIBS)
else
ifeq ($(OS),Darwin)
STD_GIMP_CFLAGS += $(X11_CFLAGS)
STD_GIMP_LIBS += $(X11_LIBS)
else
STD_GIMP_CFLAGS += $(PARALLEL_CFLAGS) $(OPENMP_CFLAGS) $(GDI32_CFLAGS) $(CURL_CFLAGS)
STD_GIMP_LIBS += -mwindows $(PARALLEL_LIBS) $(OPENMP_LIBS) $(GDI32_LIBS) $(CURL_LIBS) -lpthread
endif
endif

# Libgmic : Standard build.
ifeq ($(OS),Darwin)
STD_LIB_CFLAGS = $(MANDATORY_CFLAGS) $(FFTW_CFLAGS) $(ZLIB_CFLAGS) -Dcimg_display=0 # $(PARALLEL_CFLAGS) $(OPENMP_CFLAGS) -fno-ipa-sra
STD_LIB_LIBS = $(MANDATORY_LIBS) $(FFTW_LIBS) $(ZLIB_LIBS) # $(PARALLEL_LIBS) $(OPENMP_LIBS)
else
STD_LIB_CFLAGS = $(MANDATORY_CFLAGS) $(PARALLEL_CFLAGS) $(OPENMP_CFLAGS) $(FFTW_CFLAGS) $(ZLIB_CFLAGS) -Dcimg_display=0 # -fno-ipa-sra
STD_LIB_LIBS = $(MANDATORY_LIBS) $(PARALLEL_LIBS) $(OPENMP_LIBS) $(FFTW_LIBS) $(ZLIB_LIBS)
endif
# GMICol : Standard build.
GMICOL_UNIX_CFLAGS = $(MANDATORY_CFLAGS) $(PARALLEL_CFLAGS) $(PNG_CFLAGS) $(JPEG_CFLAGS) $(ZLIB_CFLAGS) $(FFTW_CFLAGS) -Dcimg_display=0
GMICOL_UNIX_LIBS = $(MANDATORY_LIBS) $(PARALLEL_LIBS) $(PNG_LIBS) $(JPEG_LIBS) $(ZLIB_LIBS) /usr/lib/x86_64-linux-gnu/libfftw3.a /usr/lib/x86_64-linux-gnu/libfftw3_threads.a

#--------------------------
# Define Makefile entries.
#--------------------------

# Main entries
all:
ifeq ($(OS),Unix)
	@echo "**"
	@echo "** Generic unix configuration."
	@echo "**"
	$(MAKE) linux gimp lib  # zart
else
ifeq ($(OS),Darwin)
	@echo "**"
	@echo "** MacOSX configuration."
	@echo "**"
	$(MAKE) macosx gimp  # zart
else
	@echo "**"
	@echo "** Windows configuration."
	@echo "**"
	$(MAKE) windows gimp
endif
endif

gimp:
	$(MAKE) "CFLAGS=$(STD_GIMP_CFLAGS) $(OPT_CFLAGS)" "LIBS+=$(STD_GIMP_LIBS) $(OPT_LIBS)" gmic_gimp

lib:
	$(MAKE) "CFLAGS=$(STD_LIB_CFLAGS) $(OPT_CFLAGS)" "LIBS+=$(STD_LIB_LIBS) $(OPT_LIBS)" gmic_lib

zart: lib
ifneq ($(OS),Darwin)
	cd ../zart && qmake-qt4 zart.pro && $(MAKE) && strip zart
else
	cd ../zart && qmake zart.pro && $(MAKE) "CFLAGS=$(STD_MACOSX_CFLAGS) $(OPT_CFLAGS)" "SUBLIBS=$(STD_MACOSX_LIBS) $(OPT_LIBS)"
endif

# Entries for other configurations.
linux:
	$(MAKE) "CFLAGS=$(STD_UNIX_CFLAGS) $(OPT_CFLAGS)" "LIBS=$(STD_UNIX_LIBS)" gmic

solaris:
	$(MAKE) "CFLAGS=$(STD_UNIX_CFLAGS) $(OPT_CFLAGS)" "LIBS=$(STD_UNIX_LIBS) -R$(USR)/X11R6/lib -lrt -lnsl -lsocket" gmic

macosx:
	$(MAKE) "CFLAGS=$(STD_MACOSX_CFLAGS) $(OPT_CFLAGS)" "LIBS=$(STD_MACOSX_LIBS) $(OPT_LIBS)" gmic

windows:
	$(MAKE) "CFLAGS=$(STD_WINDOWS_CFLAGS) $(OPT_CFLAGS)" "LIBS=$(STD_WINDOWS_LIBS)" gmic

gmicol:
	$(MAKE) "CFLAGS+=$(GMICOL_UNIX_CFLAGS) $(OPT_CFLAGS)" "LIBS+=$(GMICOL_UNIX_LIBS)" gmic

static:
	$(MAKE) "CFLAGS+=$(STATIC_UNIX_CFLAGS) $(OPT_CFLAGS)" "LIBS+=$(STATIC_UNIX_LIBS)" gmic

minimal:
	$(MAKE) "CFLAGS+=$(MINIMAL_UNIX_CFLAGS)" "LIBS+=$(MINIMAL_UNIX_LIBS)" gmic

winminimal:
	$(MAKE) "CFLAGS=$(MINIMAL_WINDOWS_CFLAGS)" "LIBS=$(MINIMAL_WINDOWS_LIBS)" gmic

macminimal:
	$(MAKE) "CFLAGS+=$(MINIMAL_MACOSX_CFLAGS)" "LIBS+=$(MINIMAL_MACOSX_LIBS)" gmic

custom:
	$(MAKE) "CFLAGS=$(CUSTOM_UNIX_CFLAGS) $(OPT_CFLAGS)" "LIBS=$(CUSTOM_UNIX_LIBS)" gmic

# Internal rules to build compilation modules.
gmic_lib.o: gmic.cpp
	$(CC) -o gmic_lib.o -c gmic.cpp -fPIC $(CFLAGS)
gmic_lib: gmic_lib.o
	ar rcs libgmic.a gmic_lib.o
ifneq ($(OS),Darwin)
	$(CC) -shared -Wl,-soname,libgmic.so.1 -o libgmic.so gmic_lib.o $(LIBS)
	$(CC) -o gmic_use_lib gmic_use_lib.cpp -L. -lgmic $(FFTW_LIBS)
else
	$(CC) -shared -o libgmic.so gmic_lib.o $(LIBS)
endif

gmic_gimp.o: gmic.cpp CImg.h
	$(CC) -o gmic_gimp.o -c gmic.cpp $(CFLAGS) -Dgmic_gimp
gmic_gimp : gmic_gimp.o gmic_gimp.cpp
	$(CC) -o gmic_gimp gmic_gimp.cpp gmic_gimp.o `gimptool-2.0$(EXE) --cflags` $(CFLAGS) `gimptool-2.0$(EXE) --libs` $(LIBS)
	strip gmic_gimp$(EXE)

gmic: gmic.cpp gmic.h CImg.h
	$(CC) -o gmic gmic.cpp $(CFLAGS) -Dgmic_main $(LIBS)
	strip gmic$(EXE)

gmic_def.h: gmic_def.gmic
	@echo "#ifndef gmic_gimp" > gmic_def.h
	@\gmic gmic_def.gmic raw:gmic_def.gmic,uchar -compress_gmic -o -.h,uchar | sed 's/unnamed/gmic_def/' | sed 's/ \};/, 0 \};\n\nunsigned int size_data_gmic_def = sizeof(data_gmic_def);\n/g' >> gmic_def.h
	@echo "#else" >> gmic_def.h
	@\gmic gmic_def.gmic -update_server @.,0 raw:$(HOME)/.update@..gmic,uchar -compress_gmic_gimp -o -.h,uchar | sed 's/unnamed/gmic_def/' | sed 's/ \};/, 0 \};\n\nunsigned int size_data_gmic_def = sizeof(data_gmic_def);\n/g' >> gmic_def.h
	\gmic -v - ../html/img/logoGMIC.ppm -permute cxyz -o -.h,uchar | sed 's/unnamed/gmic_logo/' >> gmic_def.h
	@echo "unsigned int size_data_logo = sizeof(data_gmic_logo);" >> gmic_def.h
	@echo "#endif" >> gmic_def.h
	@echo >>gmic_def.h

# Generate bash completion script.
bashcompletion:
	@mkdir -p ../resources
	@\gmic -v - gmic_def.gmic raw:gmic_def.gmic,uchar -document_gmic bash 2> ../resources/gmic_bashcompletion.sh

# Generate man page.
man:
	@mkdir -p ../man
	@\gmic -v - gmic_def.gmic raw:gmic_def.gmic,uchar -__help man 2> ../man/gmic.1
	@gzip -f ../man/gmic.1

# Install/uninstall/clean procedures.
install:
	mkdir -p $(DESTDIR)$(PLUGINDIR)/
	cp -f gmic_gimp $(DESTDIR)$(PLUGINDIR)/
	mkdir -p $(DESTDIR)$(USR)/$(BIN)/
	cp -f gmic $(DESTDIR)$(USR)/$(BIN)/
	mkdir -p $(DESTDIR)$(USR)/$(INCLUDE)/
	cp -f gmic.h $(DESTDIR)$(USR)/$(INCLUDE)/
ifneq ($(OS),Darwin)
	mkdir -p $(DESTDIR)$(USR)/share
	mkdir -p $(DESTDIR)$(USR)/$(LIB)
	cp -f libgmic.so $(DESTDIR)$(USR)/$(LIB)/libgmic.so._gmic_versio3
	ln -s libgmic.so._gmic_versio3 $(DESTDIR)$(USR)/$(LIB)/libgmic.so._gmic_versio1
	ln -s libgmic.so._gmic_versio1 $(DESTDIR)$(USR)/$(LIB)/libgmic.so
endif
	mkdir -p $(DESTDIR)$(USR)/share/man/
	mkdir -p $(DESTDIR)$(USR)/share/man/man1/
	mkdir -p $(DESTDIR)$(USR)/share/man/fr/man1/
	cp -f ../man/gmic.1.gz $(DESTDIR)$(USR)/share/man/man1/gmic.1.gz
	cp -f ../man/gmic.1.gz $(DESTDIR)$(USR)/share/man/fr/man1/gmic.1.gz
	if [ -d /etc/bash_completion.d/ ]; then mkdir -p $(DESTDIR)/etc/bash_completion.d/; cp -f ../resources/gmic_bashcompletion.sh $(DESTDIR)/etc/bash_completion.d/gmic; fi
	if [ -d /opt/local/etc/bash_completion.d/ ]; then mkdir -p $(DESTDIR)/opt/local/etc/bash_completion.d/; cp -f ../resources/gmic_bashcompletion.sh $(DESTDIR)/opt/local/etc/bash_completion.d/gmic; fi

uninstall:
	rm -f $(DESTDIR)$(USR)/$(BIN)/gmic
	rm -f $(DESTDIR)$(USR)/$(INCLUDE)/gmic.h
	rm -f $(DESTDIR)$(USR)/$(LIB)/libgmic.so._gmic_versio3
	rm -f $(DESTDIR)$(USR)/$(LIB)/libgmic.so._gmic_versio1
	rm -f $(DESTDIR)$(USR)/$(LIB)/libgmic.so
	rm -rf $(DESTDIR)$(USR)/share/doc/gmic/
	rm -f $(DESTDIR)$(USR)/share/man/man1/gmic.1.gz
	rm -f $(DESTDIR)$(USR)/share/man/fr/man1/gmic.1.gz

distclean: clean

clean:
	rm -rf gmic*.o gmic$(EXE) gmic_gimp$(EXE) gmic_use_lib$(EXE) libgmic* *~

# End of Makefile
