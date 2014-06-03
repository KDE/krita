include_directories( ${VECTORIMAGE_INCLUDES} ${KDE4_INCLUDES} )

set(vectorimage_LIB_SRCS    
    libemf/EmfRecords.cpp
    libemf/EmfObjects.cpp
    libemf/EmfHeader.cpp
    libemf/BitmapHeader.cpp
    libemf/Bitmap.cpp
    libemf/EmfParser.cpp
    libemf/EmfOutput.cpp
    libemf/EmfOutputDebugStrategy.cpp
    libemf/EmfOutputPainterStrategy.cpp

    libsvm/SvmStructs.cpp
    libsvm/SvmGraphicsContext.cpp
    libsvm/SvmParser.cpp
    libsvm/SvmPainterBackend.cpp
    
    libwmf/WmfStack.cpp
    libwmf/WmfDeviceContext.cpp
    libwmf/WmfParser.cpp 
    libwmf/WmfAbstractBackend.cpp 
    libwmf/WmfPainterBackend.cpp
    libwmf/WmfWriter.cpp

)

kde4_add_library(kovectorimage SHARED ${vectorimage_LIB_SRCS})

target_link_libraries(kovectorimage ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})
target_link_libraries(kovectorimage LINK_INTERFACE_LIBRARIES ${KDE4_KDECORE_LIBS})

set_target_properties(kovectorimage PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS kovectorimage ${INSTALL_TARGETS_DEFAULT_ARGS} )
