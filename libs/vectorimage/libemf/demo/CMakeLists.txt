include_directories(
	${ENHMETAFILE_SOURCE_DIR}/demo/
	${CMAKE_CURRENT_BINARY_DIR}
	${QT_INCLUDES}
)

link_directories(${ENHMETAFILE_BINARY_DIR})

set( emf_demo_bin_SRCS
    emf_demo.cpp
    EmfViewer.cpp

    ../EmfRecords.cpp
    ../EmfHeader.cpp
    ../EmfParser.cpp
    ../EmfOutput.cpp
    ../EmfOutputDebugStrategy.cpp
    ../EmfOutputPainterStrategy.cpp
)  

qt4_automoc( ${emf_demo_bin_SRCS} )
add_executable( emf_demo ${emf_demo_bin_SRCS} )

#TARGET_LINK_LIBRARIES( emf_demo libemf Qt5::Gui Qt5::Test  )
target_link_libraries( emf_demo Qt5::Gui Qt5::Test  )


