include_directories( ${KOPICTURE_INCLUDES} )

########### next target ###############

set(kopicture_LIB_SRCS
   KoPictureKey.cpp
   KoPictureBase.cpp
   KoPicture.cpp
   KoPictureShared.cpp
   KoPictureImage.cpp
   KoPictureClipart.cpp
   KoPictureEps.cpp
)

kde4_add_library(kopicture SHARED ${kopicture_LIB_SRCS})

target_link_libraries(kopicture kostore ${KDE4_KDECORE_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
target_link_libraries(kopicture LINK_INTERFACE_LIBRARIES kostore ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTCORE_LIBRARY}) 

set_target_properties(kopicture PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS kopicture ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( FILES
    KoPictureKey.h 
    KoPicture.h
    kopicture_export.h
DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

