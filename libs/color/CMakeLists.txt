if (UNIX AND NOT APPLE AND HAVE_DBUS)
    add_subdirectory(colord)
    set(kritacolor_LIB_SRCS kis_color_manager.h linux/kis_color_manager.cpp)
    set(kritacolor_EXTRA_LIBRARIES kritacolord)
    set_target_properties(kritacolord PROPERTIES
       VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION} )
elseif (MSVC)
    set(kritacolor_LIB_SRCS kis_color_manager.h dummy/kis_color_manager.cpp ${CMAKE_CURRENT_BINARY_DIR}/kritacolor_export.h)
else ()
    set(kritacolor_LIB_SRCS kis_color_manager.h dummy/kis_color_manager.cpp)
endif ()

kis_add_core_library(kritacolor SHARED ${kritacolor_LIB_SRCS} )
generate_export_header(kritacolor BASE_NAME kritacolor)

target_link_libraries(kritacolor kritaglobal ${QT_QTCORE_LIBRARY} KF5::I18n  ${kritacolor_EXTRA_LIBRARIES})

set_target_properties(kritacolor PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

install(TARGETS kritacolor  ${INSTALL_TARGETS_DEFAULT_ARGS})

