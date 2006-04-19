kde4_header()
macro_optional_find_package(PythonLibs)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${KDE4_ENABLE_EXCEPTIONS}")

add_subdirectory( api )
add_subdirectory( main )
add_subdirectory( runner )

MESSAGE(STATUS "s :PYTHON_INCLUDE_PATH :<${PYTHON_INCLUDE_PATH}")
if(PYTHON_INCLUDE_PATH)
	add_subdirectory(python)
endif(PYTHON_INCLUDE_PATH)		

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(RUBYSUBDIR)")



########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

## Compile python plugin only if python is installed. Kross itself doesn't 
## care of it cause it doesn't depend directly on the python plugin. Kross
## determinates at runtime if it's avaiable and if that's the case it just
## uses the plugin. So, we are able to don't compile the python plugin now
## and at a later stage once we like to use it, just compile+install the
## python plugin and let Kross use it without the need to recompiling whole
## Kross. So, for packagers it's recommed to move the python plugin into
## an own package that could be optional installed.
#if compile_kross_python
#  PYTHONSUBDIR = python
#endif
#
## Also only compile the ruby plugin if ruby is installed.
#if compile_kross_ruby
#  RUBYSUBDIR = ruby
#endif
#
#SUBDIRS = api main $(PYTHONSUBDIR) $(RUBYSUBDIR) runner
#
## Don't compile test by default. To use test, just cd into the directory
## and do a "make && ./krosstest" manualy.
##SUBDIRS += test
