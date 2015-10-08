set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/libs/widgetutils )

########### next target ###############

set(kopropertiestest_SRCS KoPropertiesTest.cpp )
kde4_add_unit_test(KoPropertiesTest TESTNAME libs-widgetutils-KoPropertiesTest ${kopropertiestest_SRCS})
target_link_libraries(KoPropertiesTest  kowidgetutils Qt5::Test)

########### next target ###############

set(kofiledialogtester_SRCS KoFileDialogTester.cpp)
qt5_wrap_ui(kofiledialogtester_SRCS KoFileDialogTester.ui)
add_executable(kofiledialogtester ${kofiledialogtester_SRCS})
target_link_libraries(kofiledialogtester kowidgetutils)
