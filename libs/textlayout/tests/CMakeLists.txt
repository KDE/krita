set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KOTEXT_INCLUDES} )

if(MSVC)
  # avoid "cannot open file 'LIBC.lib'" error
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBC.LIB")
endif(MSVC)

########### next target ###############

set(TestBlockLayout_test_SRCS
 TestBlockLayout.cpp
 MockRootAreaProvider.cpp
)
kde4_add_unit_test(TestBlockLayout TESTNAME libs-kotext-TestBlockLayout  ${TestBlockLayout_test_SRCS})
target_link_libraries(TestBlockLayout kotext textlayout komain ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestTableLayout_test_SRCS
 TestTableLayout.cpp
 MockRootAreaProvider.cpp
)
kde4_add_unit_test(TestTableLayout TESTNAME libs-kotext-TestTableLayout  ${TestTableLayout_test_SRCS})
target_link_libraries(TestTableLayout kotext textlayout komain ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestBorder_test_SRCS TestBorder.cpp)
kde4_add_unit_test(TestBorder TESTNAME libs-kotext-TestBorder  ${TestBorder_test_SRCS})
target_link_libraries(TestBorder kotext  ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestKoTableColumnAndRowStyleManager_test_SRCS TestKoTableColumnAndRowStyleManager.cpp)
kde4_add_unit_test(TestKoTableColumnAndRowStyleManager TESTNAME libs-kotext-KoTableColumnAndRowStyleManager ${TestKoTableColumnAndRowStyleManager_test_SRCS})
target_link_libraries(TestKoTableColumnAndRowStyleManager kotext  ${QT_QTTEST_LIBRARY})
