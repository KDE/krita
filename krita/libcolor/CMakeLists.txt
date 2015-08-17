if (UNIX AND NOT APPLE) 
    add_subdirectory(colord)
    set(kritacolor_LIB_SRCS kis_color_manager.h linux/kis_color_manager.cpp)
    set(kritacolor_EXTRA_LIBRARIES kritacolord)
else ()
    set(kritacolor_LIB_SRCS dummy/kis_color_manager.cpp)
endif () 

kde4_add_library(kritacolor SHARED ${kritacolor_LIB_SRCS} )

target_link_libraries(kritacolor ${QT_QTCORE_LIBRARY} KF5::KDELibs4Support ${kritacolor_EXTRA_LIBRARIES})

target_link_libraries(kritacolor LINK_INTERFACE_LIBRARIES ${QT_QTCORE_LIBRARY}) 

set_target_properties(kritacolor PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)

install(TARGETS kritacolor  ${INSTALL_TARGETS_DEFAULT_ARGS})

