# - Find ruby
# This module finds if RUBY is installed and determines where the include files
# and libraries are. It also determines what the name of the library is. This
# code sets the following variables:
#
#  RUBY_INCLUDE_PATH = path to where ruby.h can be found
#  RUBY_EXECUTABLE   = full path to the ruby binary
#

FIND_PROGRAM(RUBY_EXECUTABLE
  NAMES ruby ruby1.8 ruby1.9
  PATHS /usr/bin /usr/local/bin
)

MESSAGE(STATUS "s :RUBY_EXECUTABLE :<${RUBY_EXECUTABLE}")

IF (RUBY_EXECUTABLE)



    #TODO why this does not work any longer? The output-variable ${MYVAR} should
    #     contain the 'WORKS NOW !' string. The funny thing is, that it works
    #     working at least once. Hmmmm...

    #EXECUTE_PROCESS(COMMAND ruby -r rbconfig -e "'printf(\"%s\",Config::CONFIG[\"ruby_version\"])'" OUTPUT_VARIABLE _test_var)
    #EXEC_PROGRAM(${RUBY_EXECUTABLE} ARGS -e 'print \"AAAAAAA\"' OUTPUT_VARIABLE MYVAR)
    EXEC_PROGRAM(${RUBY_EXECUTABLE} ARGS "-e print 'WORKS NOW !'" OUTPUT_VARIABLE MYVAR)
    MESSAGE(STATUS "Fix Ruby detection. cmake EXEC_PROGRAM does not seem to work as expected whyever :-/ ${MYVAR}")



    EXEC_PROGRAM(${RUBY_EXECUTABLE} ARGS "-r rbconfig -e 'printf(\"%s\",Config::CONFIG[\"libdir\"])'" OUTPUT_VARIABLE EXPECTED_RUBY_LIBRARY_PATH)
    EXEC_PROGRAM(${RUBY_EXECUTABLE} ARGS "-r rbconfig -e 'printf(\"%s\",Config::CONFIG[\"rubyincludedir\"] || Config::CONFIG[\"archdir\"])'" OUTPUT_VARIABLE EXPECTED_RUBY_INCLUDE_PATH)
    EXEC_PROGRAM(${RUBY_EXECUTABLE} ARGS "-r rbconfig -e 'printf(\"%s\",Config::CONFIG[\"ruby_version\"])'" OUTPUT_VARIABLE RUBY_VERSION)

    FIND_LIBRARY(RUBY_LIBRARY
      NAMES ruby${RUBY_VERSION}
      PATHS ${EXPECTED_RUBY_LIBRARY_PATH} /usr/lib
    )

    FIND_PATH(RUBY_INCLUDE_PATH ruby.h
      ${EXPECTED_RUBY_INCLUDE_PATH}
    )

    MESSAGE(STATUS "s :RUBY_LIBRARY :<${RUBY_LIBRARY}")
    MESSAGE(STATUS "s :RUBY_INCLUDE_PATH :<${RUBY_INCLUDE_PATH}")
    MESSAGE(STATUS "s :RUBY_VERSION :<${RUBY_VERSION}")

    IF (RUBY_LIBRARY)
        IF (RUBY_INCLUDE_PATH)
            MARK_AS_ADVANCED(
              RUBY_EXECUTABLE
              RUBY_LIBRARY
              RUBY_INCLUDE_PATH
            )
        ENDIF (RUBY_INCLUDE_PATH)
    ENDIF (RUBY_LIBRARY)

ENDIF (RUBY_EXECUTABLE)
