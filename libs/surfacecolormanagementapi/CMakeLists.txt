set(kritasurfacecolormanagementapi_LIB_SRCS
    KisColorimetryUtils.cpp
    surfacecolormanagement/KisOutputColorInfoInterface.cpp
    surfacecolormanagement/KisSurfaceColorimetry.cpp
    surfacecolormanagement/KisSurfaceColorimetryIccUtils.cpp
    surfacecolormanagement/KisSurfaceColorManagerInterface.cpp
)

kis_add_library(kritasurfacecolormanagementapi SHARED ${kritasurfacecolormanagementapi_LIB_SRCS})
generate_export_header(kritasurfacecolormanagementapi BASE_NAME kritasurfacecolormanagementapi)

target_link_libraries(kritasurfacecolormanagementapi
    PRIVATE
    kritaglobal
    kritapigment
    ${QT_QTCORE_LIBRARY})

set_target_properties(kritasurfacecolormanagementapi PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

install(TARGETS kritasurfacecolormanagementapi  ${INSTALL_TARGETS_DEFAULT_ARGS})

