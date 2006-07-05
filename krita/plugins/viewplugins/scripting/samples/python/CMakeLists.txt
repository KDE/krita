




########### install files ###############

install( FILES  invert.py invertpython.rc  DESTINATION ${DATA_INSTALL_DIR}/krita/scripts/invertpython)
install( FILES  reshapehisto.py reshapehisto.rc  DESTINATION ${DATA_INSTALL_DIR}/krita/scripts/reshapehisto)





#original Makefile.am contents follow:

#scriptinvertdir = $(kde_datadir)/krita/scripts/invertpython
#scriptinvert_SCRIPTS = invert.py invertpython.rc
#
#scriptreshapedir = $(kde_datadir)/krita/scripts/reshapehisto
#scriptreshape_SCRIPTS = reshapehisto.py reshapehisto.rc
