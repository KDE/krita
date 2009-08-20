add_subdirectory( tests )

include_directories( ${KDE4_INCLUDES} )

set(kobase_LIB_SRCS
    KoProperties.cpp
    KoUnit.cpp
)

kde4_add_library(kobase SHARED ${kobase_LIB_SRCS})

target_link_libraries(kobase ${KDE4_KDECORE_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY})

target_link_libraries(kobase LINK_INTERFACE_LIBRARIES ${KDE4_KDECORE_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY})

set_target_properties(kobase PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS kobase ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(
    FILES
    KoProperties.h
    KoUnit.h
    kobase_export.h
    DESTINATION
    ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
