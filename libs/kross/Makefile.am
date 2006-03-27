# Compile python plugin only if python is installed. Kross itself doesn't 
# care of it cause it doesn't depend directly on the python plugin. Kross
# determinates at runtime if it's avaiable and if that's the case it just
# uses the plugin. So, we are able to don't compile the python plugin now
# and at a later stage once we like to use it, just compile+install the
# python plugin and let Kross use it without the need to recompiling whole
# Kross. So, for packagers it's recommed to move the python plugin into
# an own package that could be optional installed.
if compile_kross_python
  PYTHONSUBDIR = python
endif

# Also only compile the ruby plugin if ruby is installed.
if compile_kross_ruby
  RUBYSUBDIR = ruby
endif

SUBDIRS = api main $(PYTHONSUBDIR) $(RUBYSUBDIR) runner

# Don't compile test by default. To use test, just cd into the directory
# and do a "make && ./krosstest" manualy.
#SUBDIRS += test
