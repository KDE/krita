set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KOTEXT_INCLUDES} )

if(MSVC)
  # avoid "cannot open file 'LIBC.lib'" error
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBC.LIB")
endif(MSVC)

########### next target ###############

set(TestDocumentLayout_test_SRCS TestDocumentLayout.cpp)
kde4_add_unit_test(TestDocumentLayout1 TESTNAME libs-kotext-TestDocumentLayout  ${TestDocumentLayout_test_SRCS})
target_link_libraries(TestDocumentLayout1 kotext komain ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestBorder_test_SRCS TestBorder.cpp)
kde4_add_unit_test(TestBorder TESTNAME libs-kotext-TestBorder  ${TestBorder_test_SRCS})
target_link_libraries(TestBorder kotext  ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestKoTableColumnAndRowStyleManager_test_SRCS TestKoTableColumnAndRowStyleManager.cpp)
kde4_add_unit_test(TestKoTableColumnAndRowStyleManager TESTNAME libs-kotext-KoTableColumnAndRowStyleManager ${TestKoTableColumnAndRowStyleManager_test_SRCS})
target_link_libraries(TestKoTableColumnAndRowStyleManager kotext  ${QT_QTTEST_LIBRARY})
