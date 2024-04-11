add_library(kritatestsdk INTERFACE)

target_link_libraries(kritatestsdk INTERFACE Qt6::Test)

target_compile_definitions(kritatestsdk INTERFACE 
    FILES_DEFAULT_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/"
    SYSTEM_RESOURCES_DATA_DIR="${CMAKE_SOURCE_DIR}/krita/data/"
)

add_library(kritatransformmaskstubs STATIC
    KisDumbTransformMaskParams.cpp
    KisDumbAnimatedTransformMaskParamsHolder.cpp
    KritaTransformMaskStubs.cpp
)

generate_export_header(kritatransformmaskstubs BASE_NAME kritatransformmaskstubs)

target_link_libraries(kritatransformmaskstubs INTERFACE kritatestsdk)
target_link_libraries(kritatransformmaskstubs PUBLIC kritaimage)
