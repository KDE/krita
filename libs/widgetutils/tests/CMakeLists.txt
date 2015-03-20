set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/libs/widgetutils )

########### next target ###############

set(kopropertiestest_SRCS KoPropertiesTest.cpp )
add_executable(KoPropertiesTestNAME libs-widgetutils-KoPropertiesTest ${kopropertiestest_SRCS})
add_test(KoPropertiesTest KoPropertiesTest)
ecm_mark_as_test(KoPropertiesTest)
target_link_libraries(KoPropertiesTest   kowidgetutils Qt5::Test)

