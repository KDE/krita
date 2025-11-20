########### manual test for file contents ###############

add_executable(storedroptest storedroptest.cpp)
target_link_libraries(storedroptest kritastore Qt${QT_MAJOR_VERSION}::Widgets)
ecm_mark_as_test(storedroptest)

macos_test_fixrpath( storedroptest )
