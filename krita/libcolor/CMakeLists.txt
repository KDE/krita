if (UNIX AND NOT APPLE) 
    add_subdirectory(colord)
    set(kritacolor_LIB_SRCS kis_color_manager_linux.cpp)
    set(kritacolor_EXTRA_LIBRARIES kritacolord)
else ()
    set(kritacolor_LIB_SRCS kis_color_manager_dummy.cpp)
endif () 

add_library(kritacolor SHARED ${kritacolor_LIB_SRCS} )

target_link_libraries(kritacolor Qt5::Core ${kritacolor_EXTRA_LIBRARIES})

target_link_libraries(kritacolor LINK_INTERFACE_LIBRARIES Qt5::Core) 

set_target_properties(kritacolor PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)

install(TARGETS kritacolor  ${INSTALL_TARGETS_DEFAULT_ARGS})

