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
  PATHS
  /usr/bin
  /usr/local/bin
)

IF (RUBY_EXECUTABLE)

    EXEC_PROGRAM(${RUBY_EXECUTABLE} ARGS "-r rbconfig -e 'printf(\"%s\",Config::CONFIG[\"libdir\"]'" OUTPUT_VARIABLE RUBY_LIBRARY_PATH)

    EXEC_PROGRAM(${RUBY_EXECUTABLE} ARGS "-r rbconfig -e 'printf(\"%s\",Config::CONFIG[\"rubyincludedir\"])'" OUTPUT_VARIABLE RUBY_POSSIBLE_INCLUDE_PATH)
    IF (NOT RUBY_POSSIBLE_INCLUDE_PATH)
        EXEC_PROGRAM(${RUBY_EXECUTABLE} ARGS "-r rbconfig -e 'printf(\"%s\",Config::CONFIG[\"archdir\"])'" OUTPUT_VARIABLE RUBY_POSSIBLE_INCLUDE_PATH)
    ENDIF (NOT RUBY_POSSIBLE_INCLUDE_PATH)

    FIND_PATH(RUBY_INCLUDE_PATH ruby.h
      ${RUBY_POSSIBLE_INCLUDE_PATH}
    )

    FIND_LIBRARY(RUBY_LIBRARY
      NAMES ruby ruby1.8 ruby1.9
      PATHS ${RUBY_LIBRARY_PATH}
    )

    IF (RUBY_LIBRARY AND RUBY_INCLUDE_PATH)
        MARK_AS_ADVANCED(
          RUBY_EXECUTABLE
          RUBY_LIBRARY
          RUBY_INCLUDE_PATH
        )
    ENDIF (RUBY_LIBRARY AND RUBY_INCLUDE_PATH)

ENDIF (RUBY_EXECUTABLE)
