

set(kritaimageenhancement_PART_SRCS
    imageenhancement.cpp
    kis_simple_noise_reducer.cpp
    kis_wavelet_noise_reduction.cpp
    )

kde4_automoc(${kritaimageenhancement_PART_SRCS})

kde4_add_plugin(kritaimageenhancement ${kritaimageenhancement_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritaimageenhancement )

target_link_libraries(kritaimageenhancement kritaui)

install(TARGETS kritaimageenhancement  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES   DESTINATION ${DATA_INSTALL_DIR}/krita/kritaplugins)
install( FILES  kritaimageenhancement.desktop  DESTINATION ${SERVICES_INSTALL_DIR})





#original Makefile.am contents follow:

#kritarcdir = $(kde_datadir)/krita/kritaplugins
#
#kde_services_DATA = kritaimageenhancement.desktop
#
#
#INCLUDES  = -I$(srcdir)/../../../sdk \
#	-I$(srcdir)/../../../core \
#	-I$(srcdir)/../../../kritacolor/ \
#	-I$(srcdir)/../../../ui \
#	$(KOFFICE_INCLUDES) \
#	$(all_includes)
#
#kritaimageenhancement_la_SOURCES = imageenhancement.cpp \
#	kis_simple_noise_reducer.cpp kis_wavelet_noise_reduction.cpp
#
#kde_module_LTLIBRARIES = kritaimageenhancement.la
#
#kritaimageenhancement_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#
#noinst_HEADERS = imageenhancement.h kis_wavelet_noise_reduction.h
#
#kritaimageenhancement_la_LIBADD =  ../../../libkritacommon.la
#
#kritaimageenhencement_la_METASOURCES = AUTO
#
#KDE_CXXFLAGS = $(USE_EXCEPTIONS)
