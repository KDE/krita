

add_subdirectory( RestrictedPython )




########### install files ###############

install( FILES  gui.py  DESTINATION ${DATA_INSTALL_DIR}/kross/python)





#original Makefile.am contents follow:

#modulesdir = $(kde_datadir)/kross/python
#modules_SCRIPTS = gui.py
#
#SUBDIRS = RestrictedPython
