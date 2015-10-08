include_directories( ${KOWIDGETS_INCLUDES} )

set(libwmf_LIB_SRCS 
   WmfStack.cpp
   WmfDeviceContext.cpp
   WmfParser.cpp 
   WmfAbstractBackend.cpp 
   WmfPainterBackend.cpp

   WmfWriter.cpp
 )

add_library(libwmf SHARED ${libwmf_LIB_SRCS})

target_link_libraries(libwmf   Qt5::Gui)

set_target_properties(libwmf PROPERTIES VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION} )
install(TARGETS libwmf  ${INSTALL_TARGETS_DEFAULT_ARGS})

