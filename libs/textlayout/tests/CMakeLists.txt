set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KOTEXT_INCLUDES} )

if(MSVC OR (WIN32 AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel"))
  # avoid "cannot open file 'LIBC.lib'" error
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBC.LIB")
endif()

########### next target ###############

set(TestDocumentLayout_test_SRCS
 TestDocumentLayout.cpp
 MockRootAreaProvider.cpp
)
add_executable(TestDocumentLayoutNAME libs-textlayout-TestDocumentLayout ${TestDocumentLayout_test_SRCS})
add_test(TestDocumentLayout TestDocumentLayout)
ecm_mark_as_test(TestDocumentLayout)
target_link_libraries(TestDocumentLayout kotext kotextlayout Qt5::Test)

########### next target ###############

set(TestBlockLayout_test_SRCS
 TestBlockLayout.cpp
 MockRootAreaProvider.cpp
)
add_executable(TestBlockLayoutNAME libs-textlayout-TestBlockLayout  ${TestBlockLayout_test_SRCS})
add_test(TestBlockLayout TestBlockLayout)
ecm_mark_as_test(TestBlockLayout)
target_link_libraries(TestBlockLayout kotext kotextlayout Qt5::Test)

########### next target ###############

set(TestTableLayout_test_SRCS
 TestTableLayout.cpp
 MockRootAreaProvider.cpp
)
add_executable(TestTableLayoutNAME libs-textlayout-TestTableLayout  ${TestTableLayout_test_SRCS})
add_test(TestTableLayout TestTableLayout)
ecm_mark_as_test(TestTableLayout)
target_link_libraries(TestTableLayout kotext kotextlayout Qt5::Test)

########### next target ###############

set(TestKoTableColumnAndRowStyleManager_test_SRCS TestKoTableColumnAndRowStyleManager.cpp)
add_executable(TestKoTableColumnAndRowStyleManagerNAME libs-textlayout-KoTableColumnAndRowStyleManager ${TestKoTableColumnAndRowStyleManager_test_SRCS})
add_test(TestKoTableColumnAndRowStyleManager TestKoTableColumnAndRowStyleManager)
ecm_mark_as_test(TestKoTableColumnAndRowStyleManager)
target_link_libraries(TestKoTableColumnAndRowStyleManager kotext  Qt5::Test)
