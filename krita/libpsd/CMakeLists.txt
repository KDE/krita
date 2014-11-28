set(libkispsd_LIB_SRCS
    psd_utils.cpp
    psd.cpp
    compression.cpp
)

kde4_add_library(libkispsd SHARED ${libkispsd_LIB_SRCS} )

if (WIN32)
    target_link_libraries(libkispsd pigmentcms ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${WIN32_PLATFORM_NET_LIBS})
else (WIN32)
    target_link_libraries(libkispsd pigmentcms ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
endif (WIN32)

target_link_libraries(libkispsd LINK_INTERFACE_LIBRARIES ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})


set_target_properties(libkispsd PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)

install(TARGETS libkispsd  ${INSTALL_TARGETS_DEFAULT_ARGS})

