include_directories(${KOMAIN_INCLUDES} ${KRITA_INCLUDES}) 


set(kritalibkis_LIB_SRCS
    application.cpp
    document.cpp
    image.cpp
    mainwindow.cpp
    module.cpp
    node.cpp
    view.cpp
)

kde4_add_library(kritalibkis SHARED ${kritalibkis_LIB_SRCS} )

if (WIN32)
    target_link_libraries(kritalibkis kritaui kritaimage ${WIN32_PLATFORM_NET_LIBS})
else (WIN32)
    target_link_libraries(kritalibkis kritaui kritaimage)
endif (WIN32)


target_link_libraries(kritalibkis LINK_INTERFACE_LIBRARIES kritaimage kritaui)

set_target_properties(kritalibkis PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)

install(TARGETS kritalibkis  ${INSTALL_TARGETS_DEFAULT_ARGS})

