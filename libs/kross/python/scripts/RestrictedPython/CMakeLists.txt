




########### install files ###############

install( FILES  __init__.py Eval.py Guards.py Limits.py      MutatingWalker.py PrintCollector.py RCompile.py RestrictionMutator.py      SelectCompiler.py Utilities.py  DESTINATION ${DATA_INSTALL_DIR}/kross/python/RestrictedPython)





#original Makefile.am contents follow:

#restrictedpythondir = $(kde_datadir)/kross/python/RestrictedPython
#restrictedpython_SCRIPTS = __init__.py Eval.py Guards.py Limits.py \
#    MutatingWalker.py PrintCollector.py RCompile.py RestrictionMutator.py \
#    SelectCompiler.py Utilities.py
