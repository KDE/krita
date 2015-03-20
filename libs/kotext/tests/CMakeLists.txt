set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KOTEXT_INCLUDES} )

if(MSVC OR (WIN32 AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel"))
  # avoid "cannot open file 'LIBC.lib'" error
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBC.LIB")
endif()

########### next target ###############

set(TestBorder_test_SRCS TestBorder.cpp)
add_executable(TestBorderNAME libs-kotext-TestBorder  ${TestBorder_test_SRCS})
add_test(TestBorder TestBorder)
ecm_mark_as_test(TestBorder)
target_link_libraries(TestBorder kotext  Qt5::Test)

########### next target ###############

set(TestKoTableColumnAndRowStyleManager_test_SRCS TestKoTableColumnAndRowStyleManager.cpp)
add_executable(TestKoTableColumnAndRowStyleManagerNAME libs-kotext-KoTableColumnAndRowStyleManager ${TestKoTableColumnAndRowStyleManager_test_SRCS})
add_test(TestKoTableColumnAndRowStyleManager TestKoTableColumnAndRowStyleManager)
ecm_mark_as_test(TestKoTableColumnAndRowStyleManager)
target_link_libraries(TestKoTableColumnAndRowStyleManager kotext  Qt5::Test)

########### next target ###############

set(TestSection_test_SRCS TestSection.cpp)
add_executable(TestSectionNAME libs-kotext-TestSection  ${TestSection_test_SRCS})
add_test(TestSection TestSection)
ecm_mark_as_test(TestSection)
target_link_libraries(TestSection kotext  Qt5::Test)

########### next target ###############

set(TestKoTextEditor_test_SRCS TestKoTextEditor.cpp)
add_executable(TestKoTextEditorNAME libs-kotext-TestKoTextEditor  ${TestKoTextEditor_test_SRCS})
add_test(TestKoTextEditor TestKoTextEditor)
ecm_mark_as_test(TestKoTextEditor)
target_link_libraries(TestKoTextEditor kotext  Qt5::Test)

########### next target ###############

set(TestKoBookmarkManager_test_SRCS TestKoBookmarkManager.cpp)
add_executable(TestKoBookmarkManagerNAME libs-kotext-TestKoBookmarkManager  ${TestKoBookmarkManager_test_SRCS})
add_test(TestKoBookmarkManager TestKoBookmarkManager)
ecm_mark_as_test(TestKoBookmarkManager)
target_link_libraries(TestKoBookmarkManager kotext  Qt5::Test)

########### next target ###############

set(TestKoInlineTextObjectManager_test_SRCS TestKoInlineTextObjectManager.cpp)
add_executable(TestKoInlineTextObjectManagerNAME libs-kotext-TestKoInlineTextObjectManager  ${TestKoInlineTextObjectManager_test_SRCS})
add_test(TestKoInlineTextObjectManager TestKoInlineTextObjectManager)
ecm_mark_as_test(TestKoInlineTextObjectManager)
target_link_libraries(TestKoInlineTextObjectManager kotext  Qt5::Test)
