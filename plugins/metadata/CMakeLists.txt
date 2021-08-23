include_directories(${CMAKE_CURRENT_SOURCE_DIR}/common)

add_subdirectory(exif)
add_subdirectory(iptc)
add_subdirectory(xmp)

add_subdirectory(tests)
