set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KOTEXT_INCLUDES} )

if(MSVC)
  # avoid "cannot open file 'LIBC.lib'" error
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBC.LIB")
endif(MSVC)

########### next target ###############

set(TestBorder_test_SRCS TestBorder.cpp)
kde4_add_unit_test(TestBorder TESTNAME libs-kotext-TestBorder  ${TestBorder_test_SRCS})
target_link_libraries(TestBorder kotext  ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestKoTableColumnAndRowStyleManager_test_SRCS TestKoTableColumnAndRowStyleManager.cpp)
kde4_add_unit_test(TestKoTableColumnAndRowStyleManager TESTNAME libs-kotext-KoTableColumnAndRowStyleManager ${TestKoTableColumnAndRowStyleManager_test_SRCS})
target_link_libraries(TestKoTableColumnAndRowStyleManager kotext  ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestSection_test_SRCS TestSection.cpp)
kde4_add_unit_test(TestSection TESTNAME libs-kotext-TestSection  ${TestSection_test_SRCS})
target_link_libraries(TestSection kotext  ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestKoTextEditor_test_SRCS TestKoTextEditor.cpp)
kde4_add_unit_test(TestKoTextEditor TESTNAME libs-kotext-TestKoTextEditor  ${TestKoTextEditor_test_SRCS})
target_link_libraries(TestKoTextEditor kotext  ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestKoBookmarkManager_test_SRCS TestKoBookmarkManager.cpp)
kde4_add_unit_test(TestKoBookmarkManager TESTNAME libs-kotext-TestKoBookmarkManager  ${TestKoBookmarkManager_test_SRCS})
target_link_libraries(TestKoBookmarkManager kotext  ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestKoInlineTextObjectManager_test_SRCS TestKoInlineTextObjectManager.cpp)
kde4_add_unit_test(TestKoInlineTextObjectManager TESTNAME libs-kotext-TestKoInlineTextObjectManager  ${TestKoInlineTextObjectManager_test_SRCS})
target_link_libraries(TestKoInlineTextObjectManager kotext  ${QT_QTTEST_LIBRARY})
