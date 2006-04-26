# - Find ruby
# This module finds if RUBY is installed and determines where the include files
# and libraries are. It also determines what the name of the library is. This
# code sets the following variables:
#
#  RUBY_INCLUDE_PATH = path to where ruby.h can be found
#  RUBY_EXECUTABLE   = full path to the ruby binary
#

SET(RUBY_POSSIBLE_INCLUDE_PATHS
  /usr/lib/ruby/1.8/i586-linux-gnu/
  )

SET(RUBY_POSSIBLE_LIB_PATHS
  /usr/lib
  )

FIND_PATH(RUBY_INCLUDE_PATH ruby.h
  ${RUBY_POSSIBLE_INCLUDE_PATHS})

FIND_LIBRARY(RUBY_LIBRARY
  NAMES ruby
  PATHS ${RUBY_POSSIBLE_LIB_PATHS}
  )

FIND_PROGRAM(RUBY_EXECUTABLE
  NAMES ruby
  PATHS
  /usr/bin
  /usr/local/bin
)

MARK_AS_ADVANCED(
  RUBY_EXECUTABLE
  RUBY_LIBRARY
  RUBY_INCLUDE_PATH
  )

