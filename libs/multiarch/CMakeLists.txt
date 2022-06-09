kis_add_library(kritamultiarch INTERFACE)

if (HAVE_XSIMD)
    target_link_libraries(kritamultiarch INTERFACE xsimd)
endif()

set_target_properties(kritamultiarch PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
)
