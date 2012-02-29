set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${KOTEXT_INCLUDES} {CMAKE_SOURCE_DIR}/libs/main)

add_definitions(-DFILES_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/data/\\"")

########### next target ###############

set(TestLoadStyle_test_SRCS TestLoadStyle.cpp)
kde4_add_unit_test(TestLoadStyle TESTNAME kotext-opendocument-TestLoadStyle ${TestLoadStyle_test_SRCS})
target_link_libraries(TestLoadStyle kotext ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestLoading_test_SRCS TestLoading.cpp)
kde4_add_unit_test(TestLoading TESTNAME kotext-opendocument-TestLoading ${TestLoading_test_SRCS})
target_link_libraries(TestLoading kotext ${QT_QTTEST_LIBRARY}  ${QT_QTSCRIPT_LIBRARY})

########### next target ###############

set(normalizeWhiteSpace_SRCS tst_normalizeWhiteSpace.cpp)
kde4_add_unit_test(normalizeWhiteSpace TESTNAME kotext-opendocument-normalizeWhiteSpace ${normalizeWhiteSpace_SRCS})
target_link_libraries(normalizeWhiteSpace kotext ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestChangeTracking_test_SRCS TestChangeTracking.cpp)
kde4_add_unit_test(TestChangeTracking TESTNAME kotext-opendocument-TestChangeTracking ${TestChangeTracking_test_SRCS})
target_link_libraries(TestChangeTracking kotext ${QT_QTTEST_LIBRARY})
