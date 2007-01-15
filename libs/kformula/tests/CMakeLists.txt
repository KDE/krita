if( KDE4_BUILD_TESTS )

include_directories( ${CMAKE_SOURCE_DIR}/libs/kformula ${KOFFICEUI_INCLUDES} )

set( AttributeManager_test_SRCS
 TestAttributeManager.cpp )

kde4_automoc( ${AttributeManager_test_SRCS} )
kde4_add_executable( TestAttributeManager RUN_UNINSTALLED ${AttributeManager_test_SRCS} )
target_link_libraries( TestAttributeManager ${QT_QTTEST_LIBRARY} )
add_test( TestAttributeManager ${EXECUTABLE_OUTPUT_PATH}/TestAttributeManager )

endif( KDE4_BUILD_TESTS )
