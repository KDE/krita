

# Allow this to be read outside of this directory.
set(libkritasdk_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/kis_progress_subject.cc CACHE INTERNAL "")

install( FILES  kis_annotation.h      kis_canvas_controller.h      kis_canvas_observer.h      kis_canvas_subject.h      kis_global.h     kis_progress_display_interface.h      kis_progress_subject.h      kis_shared_ptr_vector.h      kis_undo_adapter.h  DESTINATION ${INCLUDE_INSTALL_DIR})



#original Makefile.am contents follow:

#KDE_OPTIONS = nofinal
#
#INCLUDES = $(KOFFICE_INCLUDES) $(KOPAINTER_INCLUDES) \
#	        $(all_includes)
#
#noinst_LTLIBRARIES = libkritasdk.la
#libkritasdk_la_SOURCES = kis_progress_subject.cc 
#libkritasdk_la_METASOURCES = AUTO
#
#include_HEADERS = \
#    kis_annotation.h \
#    kis_canvas_controller.h \
#    kis_canvas_observer.h \
#    kis_canvas_subject.h \
#    kis_global.h \
#    kis_id.h \
#    kis_integer_maths.h \
#    kis_progress_display_interface.h \
#    kis_progress_subject.h \
#    kis_shared_ptr_vector.h \
#    kis_undo_adapter.h \
#    kis_generic_registry.h 
#
