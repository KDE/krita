kde4_header()

include_directories( ${KOFFICECORE_INCLUDES} )


########### next target ###############

set(kwmf_LIB_SRCS kwmf.cc qwmf.cc )

kde4_automoc(${kwmf_LIB_SRCS})

kde4_add_library(kwmf SHARED ${kwmf_LIB_SRCS})

target_link_libraries(kwmf  ${KDE4_KDECORE_LIBS} )

set_target_properties(kwmf PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} kwmf )


########### next target ###############

set(kowmf_LIB_SRCS 
   kowmfreadprivate.cc 
   kowmfstack.cc 
   kowmfread.cc 
   kowmfwrite.cc 
   kowmfpaint.cc )

kde4_automoc(${kowmf_LIB_SRCS})

kde4_add_library(kowmf SHARED ${kowmf_LIB_SRCS})

target_link_libraries(kowmf  ${KDE4_KDECORE_LIBS} )

set_target_properties(kowmf PROPERTIES VERSION 2.0.0 SOVERSION 2 )
install_targets(${LIB_INSTALL_DIR} kowmf )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

######## General stuff
#
#INCLUDES= $(KOFFICECORE_INCLUDES) -I$(srcdir) $(all_includes)
#libkwmf_la_LDFLAGS = $(all_libraries) -version-info 3:0:0 -no-undefined
## We use kdecore for kdDebug :)
#libkwmf_la_LIBADD = $(LIB_KDECORE)
#
######## Files
#
#lib_LTLIBRARIES = libkwmf.la libkowmf.la
#
#libkwmf_la_SOURCES = kwmf.cc qwmf.cc
#
##include_HEADERS = kowmfread.h kowmfwrite.h kowmfpaint.h
#noinst_HEADERS = kwmf.h qwmf.h metafuncs.h wmfstruct.h kowmfreadprivate.h kowmfstack.h kowmfstruct.h
#
#libkowmf_la_SOURCES  = kowmfreadprivate.cc kowmfstack.cc kowmfread.cc kowmfwrite.cc kowmfpaint.cc
#libkowmf_la_LDFLAGS = $(all_libraries) -version-info 2:0:0  -no-undefined
#libkowmf_la_LIBADD = $(LIB_KDECORE)
