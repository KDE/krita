add_library(kritatestsdk INTERFACE)

target_link_libraries(kritatestsdk INTERFACE Qt5::Test)

target_compile_definitions(kritatestsdk INTERFACE 
    FILES_DEFAULT_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/"
    SYSTEM_RESOURCES_DATA_DIR="${CMAKE_SOURCE_DIR}/krita/data/"
)
