set(module_install_dir ${DATA_INSTALL_DIR}/cmake/modules )

configure_file( FindCalligraLibs.cmake.in "${CMAKE_BINARY_DIR}/FindCalligraLibs.cmake" @ONLY)

install( FILES ${CMAKE_BINARY_DIR}/FindCalligraLibs.cmake DESTINATION ${module_install_dir} )
