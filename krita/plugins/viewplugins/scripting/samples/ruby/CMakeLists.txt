




########### install files ###############

install_files( ${DATA_INSTALL_DIR}/krita/scripts/torturepainting FILES  torture-painting.rb torture-painting.rc )
install_files( ${DATA_INSTALL_DIR}/krita/scripts/changecs FILES  changecs.rb changecs.rc )
install_files( ${DATA_INSTALL_DIR}/krita/scripts/invertruby FILES  invert.rb invertruby.rc )
install_files( ${DATA_INSTALL_DIR}/krita/scripts/torturefilters FILES  torture-filters.rb torture-filters.rc )
install_files( ${DATA_INSTALL_DIR}/krita/scripts/randompaint FILES  randompaint.rb randompaint.rc )
install_files( ${DATA_INSTALL_DIR}/krita/scripts/filterstest FILES  filterstest.rb filterstest.rc )





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
