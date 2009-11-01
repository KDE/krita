

include_directories( ${KOMAIN_INCLUDES} )

#add_subdirectory(demo)


########### library target ###############

set(qemf_LIB_SRCS 
     EmfRecords.cpp
     EmfHeader.cpp
     EmfParser.cpp
     EmfOutput.cpp
)

kde4_add_library(qemf SHARED ${qemf_LIB_SRCS})

target_link_libraries(qemf  ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} )

set_target_properties(qemf PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )


########### install files ###############

install(TARGETS qemf  ${INSTALL_TARGETS_DEFAULT_ARGS})

install(
    FILES
    EmfEnums.h
    EmfRecords.h
    EmfHeader.h
    EmfParser.h
    EmfOutput.h
    emf_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
