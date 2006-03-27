kde4_header()




########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kross/python/RestrictedPython FILES  __init__.py Eval.py Guards.py Limits.py      MutatingWalker.py PrintCollector.py RCompile.py RestrictionMutator.py      SelectCompiler.py Utilities.py )

kde4_footer()



#original Makefile.am contents follow:

#restrictedpythondir = $(kde_datadir)/kross/python/RestrictedPython
#restrictedpython_SCRIPTS = __init__.py Eval.py Guards.py Limits.py \
#    MutatingWalker.py PrintCollector.py RCompile.py RestrictionMutator.py \
#    SelectCompiler.py Utilities.py
