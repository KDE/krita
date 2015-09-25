include_directories( ${KOMAIN_INCLUDES} )


########### library target ###############

set(libemf_LIB_SRCS 
     EmfRecords.cpp
     EmfObjects.cpp
     EmfHeader.cpp
     BitmapHeader.cpp
     EmfParser.cpp
     EmfOutput.cpp
     EmfOutputDebugStrategy.cpp
     EmfOutputPainterStrategy.cpp
)
