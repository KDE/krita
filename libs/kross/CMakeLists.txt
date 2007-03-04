include_directories( ${KOFFICECORE_INCLUDES} )

#####################################################################
# Kross Python
#
# Install examples:
#   cmake -DCMAKE_INSTALL_PREFIX=/opt/kde4/ ..
#   cmake -DCMAKE_INSTALL_PREFIX=/opt/kde4/ -DENABLE_KROSSPYTHON=off
#   cmake -DCMAKE_INSTALL_PREFIX=/opt/kde4/ -DCMAKE_BUILD_TYPE=debugfull -DPYTHON_LIBRARY=/usr/lib/python2.5/config/libpython2.5.so -DPYTHON_LIBRARIES=/usr/lib/python2.5/config/libpython2.5.so -DPYTHON_INCLUDE_PATH=/usr/include/python2.5 ..

OPTION(ENABLE_KROSSPYTHON "build Python scripting library (Kross)" ON)
#OPTION(ENABLE_KROSSPYTHON "build Python scripting library (Kross)" OFF)

set(KROSSPYTHON_ENABLED "no")
if(ENABLE_KROSSPYTHON)
    if(PYTHON_LIBRARIES AND PYTHON_INCLUDE_PATH)
        add_subdirectory(python)
        set(KROSSPYTHON_ENABLED "yes")
    endif(PYTHON_LIBRARIES AND PYTHON_INCLUDE_PATH)
    message(STATUS "Python Libraries: " ${PYTHON_LIBRARIES})
    message(STATUS "Python Include Path: " ${PYTHON_INCLUDE_PATH})
endif(ENABLE_KROSSPYTHON)
message(STATUS "Compile Kross Python... " ${KROSSPYTHON_ENABLED})

#####################################################################
# Kross Ruby
#
# Install examples:
#   cmake -DCMAKE_INSTALL_PREFIX=/opt/kde4/ ..
#   cmake -DCMAKE_INSTALL_PREFIX=/opt/kde4/ -DENABLE_KROSSRUBY=off
#   cmake -DCMAKE_INSTALL_PREFIX=/opt/kde4/ -DCMAKE_BUILD_TYPE=debugfull -DKOFFICE_ENABLE_SCRIPTING=y -DRUBY_INCLUDE_PATH=/usr/lib/ruby/1.8/i486-linux/ -DRUBY_LIBRARY=/usr/lib/libruby1.8.so ..

OPTION(ENABLE_KROSSRUBY "build Ruby scripting library (Kross)" ON)
#OPTION(ENABLE_KROSSRUBY "build Ruby scripting library (Kross)" OFF)

set(KROSSRUBY_ENABLED "no")
if(ENABLE_KROSSRUBY)
    macro_optional_find_package(RUBY)
    if (RUBY_EXECUTABLE AND RUBY_LIBRARY AND RUBY_INCLUDE_PATH)
        EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r mkmf -e "exit(have_func('rb_hash_foreach') ? 0 : 1)" RESULT_VARIABLE RUBY_HASH_FOREACH OUTPUT_QUIET)
        if (RUBY_HASH_FOREACH EQUAL 0)
            add_subdirectory(ruby)
            set(KROSSRUBY_ENABLED "yes")
        else (RUBY_HASH_FOREACH EQUAL 0)

            # 2007-02-12 sebsauer; this is a workaround for the case we failed to load
            # the "mkmf" library what is the case e.g. on some debian distributions.
            #EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r mkmf -e "exit(0)" RESULT_VARIABLE RUBY_HASH_FOREACH OUTPUT_QUIET ERROR_QUIET)
            #if (${RUBY_HASH_FOREACH} EQUAL 0)
            message(STATUS "The detected Ruby is too old for kross. A Ruby which has rb_hash_foreach is required. Upgrade to 1.8.2 or later")
            #else (${RUBY_HASH_FOREACH} EQUAL 0)
            #    message(STATUS "Failed to load the Ruby mkmf library. Please install the Ruby developer packages.")
            #endif (${RUBY_HASH_FOREACH} EQUAL 0)

        endif (RUBY_HASH_FOREACH EQUAL 0)
    endif (RUBY_EXECUTABLE AND RUBY_LIBRARY AND RUBY_INCLUDE_PATH)
    message(STATUS "Ruby Executable: " ${RUBY_EXECUTABLE})
    message(STATUS "Ruby Libraries: " ${RUBY_LIBRARY})
    message(STATUS "Ruby Include Path: " ${RUBY_INCLUDE_PATH})
endif(ENABLE_KROSSRUBY)
message(STATUS "Compile Kross Ruby... " ${KROSSRUBY_ENABLED})
