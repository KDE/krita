if (UNIX AND NOT APPLE) 
    add_subdirectory(colord)
    set(kritacolor_LIB_SRCS kis_color_manager.h linux/kis_color_manager.cpp)
    set(kritacolor_EXTRA_LIBRARIES kritacolord)
elseif (MSVC)
    set(kritacolor_LIB_SRCS kis_color_manager.h dummy/kis_color_manager.cpp ${CMAKE_CURRENT_BINARY_DIR}/kritacolor_export.h)
else ()
    set(kritacolor_LIB_SRCS kis_color_manager.h dummy/kis_color_manager.cpp)
endif () 

add_library(kritacolor SHARED ${kritacolor_LIB_SRCS} )
generate_export_header(kritacolor BASE_NAME kritacolor)

target_link_libraries(kritacolor kritaglobal ${QT_QTCORE_LIBRARY} KF5::I18n  ${kritacolor_EXTRA_LIBRARIES})

target_link_libraries(kritacolor LINK_INTERFACE_LIBRARIES kritaglobal ${QT_QTCORE_LIBRARY}) 

set_target_properties(kritacolor PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)

install(TARGETS kritacolor  ${INSTALL_TARGETS_DEFAULT_ARGS})

