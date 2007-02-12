include_directories( ${KOFFICECORE_INCLUDES} )

#####################################################################
# Kross Python

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

OPTION(ENABLE_KROSSRUBY "build Ruby scripting library (Kross)" ON)
#OPTION(ENABLE_KROSSRUBY "build Ruby scripting library (Kross)" OFF)

set(KROSSRUBY_ENABLED "no")
if(ENABLE_KROSSRUBY)
    macro_optional_find_package(RUBY)
    if (RUBY_EXECUTABLE AND RUBY_LIBRARY AND RUBY_INCLUDE_PATH)
        EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r mkmf -e "exit(have_func('rb_hash_foreach') ? 0 : 1)" RESULT_VARIABLE RUBY_HASH_FOREACH OUTPUT_QUIET)
        if (${RUBY_HASH_FOREACH} EQUAL 0)
            add_subdirectory(ruby)
            set(KROSSRUBY_ENABLED "yes")
        else (${RUBY_HASH_FOREACH} EQUAL 0)
            message(STATUS "The detected Ruby is too old for kross. A Ruby which has rb_hash_foreach is required. Upgrade to 1.8.2 or later")
        endif (${RUBY_HASH_FOREACH} EQUAL 0)
    endif (RUBY_EXECUTABLE AND RUBY_LIBRARY AND RUBY_INCLUDE_PATH)
    message(STATUS "Ruby Executable: " ${RUBY_EXECUTABLE})
    message(STATUS "Ruby Libraries: " ${RUBY_LIBRARY})
    message(STATUS "Ruby Include Path: " ${RUBY_INCLUDE_PATH})
endif(ENABLE_KROSSRUBY)
message(STATUS "Compile Kross Ruby... " ${KROSSRUBY_ENABLED})
