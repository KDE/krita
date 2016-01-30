include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/libemf
    ${CMAKE_CURRENT_SOURCE_DIR}/libsvm
    ${CMAKE_CURRENT_SOURCE_DIR}/libwmf
)

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

target_include_directories(kritavectorimage
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libemf>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libwmf>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libsvm>
)


target_link_libraries(kritavectorimage  KF5::I18n Qt5::Gui Qt5::PrintSupport)

set_target_properties(kritavectorimage PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS kritavectorimage ${INSTALL_TARGETS_DEFAULT_ARGS} )
