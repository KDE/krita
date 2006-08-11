

add_subdirectory( hyphdicts )

include_directories( ${KOFFICECORE_INCLUDES} )

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${KDE4_ENABLE_EXCEPTIONS}")



########### next target ###############

if(KDE4_BUILD_TESTS)

set(kohyphentest_SRCS kohyphentest.cpp )

kde4_automoc(${kohyphentest_SRCS})

kde4_add_executable(kohyphentest RUN_UNINSTALLED ${kohyphentest_SRCS})

target_link_libraries(kohyphentest kotext )

endif(KDE4_BUILD_TESTS)

