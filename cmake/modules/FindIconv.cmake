# - Try to find Iconv 
# Once done this will define 
# 
#  ICONV_FOUND - system has Iconv 
#  ICONV_INCLUDE_DIR - the Iconv include directory 
#  ICONV_LIBRARIES - Link these to use Iconv 
#  ICONV_SECOND_ARGUMENT_IS_CONST - the second argument for iconv() is const
# 
include(CheckCXXSourceCompiles)

if (ICONV_INCLUDE_DIR AND ICONV_LIBRARIES)
  # Already in cache, be silent
  set(ICONV_FIND_QUIETLY TRUE)
endif ()

find_path(ICONV_INCLUDE_DIR iconv.h) 
 
find_library(ICONV_LIBRARIES NAMES iconv libiconv libiconv-2 c)
 
if(ICONV_INCLUDE_DIR AND ICONV_LIBRARIES) 
   set(ICONV_FOUND TRUE) 
endif() 

set(CMAKE_REQUIRED_INCLUDES ${ICONV_INCLUDE_DIR})
set(CMAKE_REQUIRED_LIBRARIES ${ICONV_LIBRARIES})
if(ICONV_FOUND)
  check_cxx_source_compiles("
  #include <iconv.h>
  int main(){
    iconv_t conv = 0;
    const char* in = 0;
    size_t ilen = 0;
    char* out = 0;
    size_t olen = 0;
    iconv(conv, &in, &ilen, &out, &olen);
    return 0;
  }
" ICONV_SECOND_ARGUMENT_IS_CONST )
endif()
set(CMAKE_REQUIRED_INCLUDES)
set(CMAKE_REQUIRED_LIBRARIES)

if(ICONV_FOUND) 
  if(NOT ICONV_FIND_QUIETLY) 
    message(STATUS "Found Iconv: ${ICONV_LIBRARIES}") 
  endif() 
else() 
  if(Iconv_FIND_REQUIRED) 
    message(FATAL_ERROR "Could not find Iconv") 
  endif() 
endif() 

mark_as_advanced(
  ICONV_INCLUDE_DIR
  ICONV_LIBRARIES
  ICONV_SECOND_ARGUMENT_IS_CONST
)
