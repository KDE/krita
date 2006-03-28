kde4_header()

add_subdirectory( RestrictedPython )




########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kross/python FILES  gui.py )

kde4_footer()



#original Makefile.am contents follow:

#modulesdir = $(kde_datadir)/kross/python
#modules_SCRIPTS = gui.py
#
#SUBDIRS = RestrictedPython
