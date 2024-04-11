set(kritamultiarch_SRCS
    KisSupportedArchitectures.cpp
)

kis_add_library(kritamultiarch SHARED ${kritamultiarch_SRCS})

generate_export_header(kritamultiarch)

if (HAVE_XSIMD)
    target_link_libraries(kritamultiarch PUBLIC xsimd)
endif()

target_link_libraries(kritamultiarch
    PUBLIC
        Qt6::Core
    PRIVATE
        KF6::ConfigCore
        kritaglobal
)

set_target_properties(kritamultiarch PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

target_include_directories(kritamultiarch
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
)

install(TARGETS kritamultiarch ${INSTALL_TARGETS_DEFAULT_ARGS})
