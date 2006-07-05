




########### install files ###############

install( FILES  torture-painting.rb torture-painting.rc  DESTINATION ${DATA_INSTALL_DIR}/krita/scripts/torturepainting)
install( FILES  changecs.rb changecs.rc  DESTINATION ${DATA_INSTALL_DIR}/krita/scripts/changecs)
install( FILES  invert.rb invertruby.rc  DESTINATION ${DATA_INSTALL_DIR}/krita/scripts/invertruby)
install( FILES  torture-filters.rb torture-filters.rc  DESTINATION ${DATA_INSTALL_DIR}/krita/scripts/torturefilters)
install( FILES  randompaint.rb randompaint.rc  DESTINATION ${DATA_INSTALL_DIR}/krita/scripts/randompaint)
install( FILES  filterstest.rb filterstest.rc  DESTINATION ${DATA_INSTALL_DIR}/krita/scripts/filterstest)





#original Makefile.am contents follow:

#scriptsinvertdir = $(kde_datadir)/krita/scripts/invertruby
#scriptsinvert_SCRIPTS = invert.rb invertruby.rc
#
#scriptschangecsdir = $(kde_datadir)/krita/scripts/changecs
#scriptschangecs_SCRIPTS = changecs.rb changecs.rc
#
#scriptsrandompaintdir = $(kde_datadir)/krita/scripts/randompaint
#scriptsrandompaint_SCRIPTS = randompaint.rb randompaint.rc
#
#scriptsfilterstestdir = $(kde_datadir)/krita/scripts/filterstest
#scriptsfilterstest_SCRIPTS = filterstest.rb filterstest.rc
#
#scriptstorturefiltersdir = $(kde_datadir)/krita/scripts/torturefilters
#scriptstorturefilters_SCRIPTS = torture-filters.rb torture-filters.rc
#
#scriptstorturepaintingdir = $(kde_datadir)/krita/scripts/torturepainting
#scriptstorturepainting_SCRIPTS = torture-painting.rb torture-painting.rc
#
