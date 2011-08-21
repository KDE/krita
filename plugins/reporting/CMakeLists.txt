set(KPLATO_INCLUDES
    ${KPLATO_INCLUDES}
    ${CMAKE_SOURCE_DIR}/kplato/libs/ui/reports
    ${CMAKE_BINARY_DIR}/kplato/libs/ui/reports
    ${CMAKE_SOURCE_DIR}/kplato/libs/kernel
    ${CMAKE_SOURCE_DIR}/kplato/libs/models
    ${CMAKE_BINARY_DIR}/kplato/libs/models
    ${CMAKE_SOURCE_DIR}/kplato/libs/ui
    ${CMAKE_BINARY_DIR}/kplato/libs/ui
    ${CMAKE_BINARY_DIR}/kplato
    ${CMAKE_SOURCE_DIR}/kdgantt
    ${CMAKE_BINARY_DIR}/kdgantt
    ${CMAKE_SOURCE_DIR}/plugins/chartshape/kdchart/include

    ${KOMAIN_INCLUDES}
    ${KDEPIMLIBS_INCLUDE_DIR}

    ${CMAKE_SOURCE_DIR}/libs/koreport
    ${CMAKE_SOURCE_DIR}/libs/koreport/common
    ${CMAKE_SOURCE_DIR}/libs/koreport/renderer
    ${CMAKE_SOURCE_DIR}/libs/koreport/wrtembed
)

add_subdirectory(barcode)
add_subdirectory(chart)

#an unfinished plugin
add_subdirectory(shape)

if(MARBLE_FOUND)
  macro_optional_add_subdirectory(maps)
endif(MARBLE_FOUND)
