set(kritasurfacecolormanagementapi_LIB_SRCS
    KisColorimetryUtils.cpp
    surfacecolormanagement/KisOutputColorInfoInterface.cpp
    surfacecolormanagement/KisOutputColorInfoInterface.h
    surfacecolormanagement/KisSurfaceColorimetry.cpp
    surfacecolormanagement/KisSurfaceColorimetry.h
    surfacecolormanagement/KisSurfaceColorManagerInterface.cpp
    surfacecolormanagement/KisSurfaceColorManagerInterface.h
)

kis_add_library(kritasurfacecolormanagementapi SHARED ${kritasurfacecolormanagementapi_LIB_SRCS})
generate_export_header(kritasurfacecolormanagementapi BASE_NAME kritasurfacecolormanagementapi)

target_link_libraries(kritasurfacecolormanagementapi kritaglobal ${QT_QTCORE_LIBRARY})

set_target_properties(kritasurfacecolormanagementapi PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

install(TARGETS kritasurfacecolormanagementapi  ${INSTALL_TARGETS_DEFAULT_ARGS})

