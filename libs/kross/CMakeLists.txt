include_directories( ${KOFFICECORE_INCLUDES} )
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${KDE4_ENABLE_EXCEPTIONS}")

macro_optional_find_package(RUBY)

# Python
if(PYTHON_LIBRARIES AND PYTHON_INCLUDE_PATH)
	add_subdirectory(python)
endif(PYTHON_LIBRARIES AND PYTHON_INCLUDE_PATH)

# Ruby

if (RUBY_EXECUTABLE)
  EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r mkmf -e "have_func('rb_hash_foreach')" OUTPUT_VARIABLE RUBY_HASH_FOREACH)
  STRING(REPLACE "\n" "" RUBY_HASH_FOREACH "${RUBY_HASH_FOREACH}")
  message(STATUS ${RUBY_HASH_FOREACH})

  if ("${RUBY_HASH_FOREACH}" STREQUAL "checking for rb_hash_foreach()... yes")
    set (RUBY_RECENT_ENOUGH TRUE)
  else ("${RUBY_HASH_FOREACH}" STREQUAL "checking for rb_hash_foreach()... yes")
    message(STATUS "The detected Ruby is too old for kross. A Ruby which has rb_hash_foreach is required. Upgrade to 1.8.2 or later")
  endif ("${RUBY_HASH_FOREACH}" STREQUAL "checking for rb_hash_foreach()... yes")

  if(RUBY_LIBRARY AND RUBY_INCLUDE_PATH AND RUBY_RECENT_ENOUGH)
	add_subdirectory(ruby)
  endif(RUBY_LIBRARY AND RUBY_INCLUDE_PATH AND RUBY_RECENT_ENOUGH)
endif (RUBY_EXECUTABLE)
