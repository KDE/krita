INCLUDE_DIRECTORIES(
	${ENHMETAFILE_SOURCE_DIR}/demo/
	${CMAKE_CURRENT_BINARY_DIR}
	${QT_INCLUDES}
)

LINK_DIRECTORIES(${ENHMETAFILE_BINARY_DIR})

SET( emf_demo_bin_SRCS
    emf_demo.cpp
    EmfViewer.cpp

    ../EmfRecords.cpp
    ../EmfHeader.cpp
    ../EmfParser.cpp
    ../EmfOutput.cpp
    ../EmfOutputDebugStrategy.cpp
    ../EmfOutputPainterStrategy.cpp
)  

QT4_AUTOMOC( ${emf_demo_bin_SRCS} )
ADD_EXECUTABLE( emf_demo ${emf_demo_bin_SRCS} )

#TARGET_LINK_LIBRARIES( emf_demo libemf ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY}  )
TARGET_LINK_LIBRARIES( emf_demo ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY}  )


