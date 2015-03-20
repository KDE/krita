enable_testing()

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
########### AttributeManager Test ###############

 set( TestAttributeManager_SRCS TestAttributeManager.cpp )
 add_executable( TestAttributeManagerNAME kformula-AttributeManager  ${TestAttributeManager_SRCS} )
add_test(TestAttributeManager TestAttributeManager)
ecm_mark_as_test(TestAttributeManager)
 target_link_libraries( TestAttributeManager kformula Qt5::Test )

########### Load Test ###############

set(TestLoad_SRCS TestLoad.cpp)
add_executable(TestLoadNAME kformula-Load  ${TestLoad_SRCS})
add_test(TestLoad TestLoad)
ecm_mark_as_test(TestLoad)
target_link_libraries(TestLoad kformula Qt5::Test)

########### Load and Save Test ###############

set(TestLoadAndSave_SRCS TestLoadAndSave.cpp)
add_executable(TestLoadAndSaveNAME kformula-LoadAndSave  ${TestLoadAndSave_SRCS})
add_test(TestLoadAndSave TestLoadAndSave)
ecm_mark_as_test(TestLoadAndSave)
target_link_libraries(TestLoadAndSave kformula Qt5::Test)

########### Layout Test ###############

#set(TestLayout_SRCS TestLayout.cpp)
#add_executable(TestLayoutNAME kformula-Layout  ${TestLayout_SRCS})
add_test(TestLayout TestLayout)
ecm_mark_as_test(TestLayout)
#target_link_libraries(TestLayout kformula Qt5::Test)

########### Cursor Test ###############

set(TestCursor_SRCS
TestCursor.cpp
   ../FormulaPart.cpp
   ../KoFormulaShape.cpp
   ../FormulaDocument.cpp
   ../FormulaToolWidget.cpp
   ../FormulaCommandUpdate.cpp
   ../KoFormulaShapeFactory.cpp
   ../KoFormulaTool.cpp
   ../FormulaCommand.cpp
)
if(NOT WIN32)
set(TestCursor_SRCS
    ${TestCursor_SRCS}
    ../FormulaCursor.cpp)
endif()

ki18n_wrap_ui( TestCursor_SRCS
    ../FormulaToolWidget.ui
)

add_executable(TestCursorNAME kformula-Cursor  ${TestCursor_SRCS})
add_test(TestCursor TestCursor)
ecm_mark_as_test(TestCursor)
target_link_libraries(TestCursor kformula Qt5::Test)
