

include_directories( ${KOMAIN_INCLUDES} )


########### next target ###############

#set(kwmf_LIB_SRCS kwmf.cc qwmf.cc )
#
#
#kde4_add_library(kwmf SHARED ${kwmf_LIB_SRCS})
#
#target_link_libraries(kwmf  ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})
#
#set_target_properties(kwmf PROPERTIES VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION} )
#install(TARGETS kwmf  ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(libwmf_LIB_SRCS 
   WmfStack.cpp
   WmfDeviceContext.cpp
   WmfParser.cpp 
   WmfAbstractBackend.cpp 
   WmfPainterBackend.cpp

   WmfWriter.cpp
 )

kde4_add_library(libwmf SHARED ${libwmf_LIB_SRCS})

target_link_libraries(libwmf  ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})

set_target_properties(libwmf PROPERTIES VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION} )
install(TARGETS libwmf  ${INSTALL_TARGETS_DEFAULT_ARGS})

