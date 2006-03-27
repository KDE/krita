# install the cmake files

file(GLOB cmakeFiles "${CMAKE_CURRENT_SOURCE_DIR}/*.cmake")

install_files(/share/apps/cmake/modules FILES  ${cmakeFiles} )

