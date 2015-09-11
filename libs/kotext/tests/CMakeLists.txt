set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KOTEXT_INCLUDES} )

if(MSVC OR (WIN32 AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel"))
  # avoid "cannot open file 'LIBC.lib'" error
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBC.LIB")
endif()

########### next target ###############

set(TestBorder_test_SRCS TestBorder.cpp)
kde4_add_unit_test(TestBorder TESTNAME libs-kotext-TestBorder  ${TestBorder_test_SRCS})
target_link_libraries(TestBorder kotext  Qt5::Test)

########### next target ###############

set(TestKoTableColumnAndRowStyleManager_test_SRCS TestKoTableColumnAndRowStyleManager.cpp)
kde4_add_unit_test(TestKoTableColumnAndRowStyleManager TESTNAME libs-kotext-KoTableColumnAndRowStyleManager ${TestKoTableColumnAndRowStyleManager_test_SRCS})
target_link_libraries(TestKoTableColumnAndRowStyleManager kotext  Qt5::Test)

########### next target ###############

set(TestKoTextEditor_test_SRCS TestKoTextEditor.cpp)
kde4_add_unit_test(TestKoTextEditor TESTNAME libs-kotext-TestKoTextEditor  ${TestKoTextEditor_test_SRCS})
target_link_libraries(TestKoTextEditor kotext  Qt5::Test)

########### next target ###############

set(TestKoBookmarkManager_test_SRCS TestKoBookmarkManager.cpp)
kde4_add_unit_test(TestKoBookmarkManager TESTNAME libs-kotext-TestKoBookmarkManager  ${TestKoBookmarkManager_test_SRCS})
target_link_libraries(TestKoBookmarkManager kotext  Qt5::Test)

########### next target ###############

set(TestKoInlineTextObjectManager_test_SRCS TestKoInlineTextObjectManager.cpp)
kde4_add_unit_test(TestKoInlineTextObjectManager TESTNAME libs-kotext-TestKoInlineTextObjectManager  ${TestKoInlineTextObjectManager_test_SRCS})
target_link_libraries(TestKoInlineTextObjectManager kotext  Qt5::Test)
