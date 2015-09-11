

add_subdirectory( hyphdicts )

if(BUILD_TESTING)
include_directories( ${KOMAIN_INCLUDES} )

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${KDE4_ENABLE_EXCEPTIONS}")



########### next target ###############

set(kohyphentest_SRCS kohyphentest.cpp )


add_executable(kohyphentest RUN_UNINSTALLED ${kohyphentest_SRCS})

target_link_libraries(kohyphentest kotext )

endif()

