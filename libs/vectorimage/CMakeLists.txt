include_directories( ${VECTORIMAGE_INCLUDES} ${KDE4_INCLUDES} )

include_directories(${KOMAIN_INCLUDES})

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

kde4_add_library(vectorimage SHARED ${vectorimage_LIB_SRCS})

target_link_libraries(vectorimage ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})
target_link_libraries(vectorimage LINK_INTERFACE_LIBRARIES ${KDE4_KDECORE_LIBS})

set_target_properties(vectorimage PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS vectorimage ${INSTALL_TARGETS_DEFAULT_ARGS} )

