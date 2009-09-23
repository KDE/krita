set(module_install_dir ${DATA_INSTALL_DIR}/cmake/modules )

configure_file( FindKOfficeLibs.cmake.in "${CMAKE_BINARY_DIR}/FindKOfficeLibs.cmake" @ONLY)

install( FILES ${CMAKE_BINARY_DIR}/FindKOfficeLibs.cmake
         DESTINATION ${module_install_dir}  )
