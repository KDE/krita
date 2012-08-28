project( vectorshape)

# The EMF library is directly under this directory.
# When it becomes an entity on its own we have to create
# some cmake code here to find it.
SET(EMF_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/libemf)

# Duplicate from filters/CMakeLists.txt
set(WMF_INCLUDES ${CMAKE_SOURCE_DIR}/filters/libkowmf)

include_directories(${KOMAIN_INCLUDES} ${FLAKE_INCLUDES} ${WMF_INCLUDES} ${EMF_INCLUDES})

SET ( VectorShape_SRCS
    VectorShapePlugin.cpp
    VectorShape.cpp
    VectorShapeConfigWidget.cpp
    VectorShapeFactory.cpp
    #VectorCollection.cpp
    #VectorData.cpp
    VectorTool.cpp
    VectorToolFactory.cpp
    ChangeVectorDataCommand.cpp

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
)

#kde4_add_ui_files(VectorShape_SRCS
#)

kde4_add_plugin(vectorshape ${VectorShape_SRCS})

target_link_libraries(vectorshape flake libwmf ${KDE4_KFILE_LIBRARY})

install(TARGETS vectorshape DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES vectorshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})
#kde4_install_icons( ${DATA_INSTALL_DIR}/calligra/icons )


########### subdirectories ###############

#add_subdirectory( libemf )
