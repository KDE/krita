include_directories( ${VECTORIMAGE_INCLUDES}  )

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

    VectorImageDebug.cpp
)

add_library(kritavectorimage SHARED ${vectorimage_LIB_SRCS})
generate_export_header(kritavectorimage BASE_NAME kritavectorimage)

target_link_libraries(kritavectorimage  KF5::I18n Qt5::Gui Qt5::PrintSupport)
target_link_libraries(kritavectorimage LINK_INTERFACE_LIBRARIES )

set_target_properties(kritavectorimage PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS kritavectorimage ${INSTALL_TARGETS_DEFAULT_ARGS} )
