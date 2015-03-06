

include_directories( ${KOMAIN_INCLUDES} )

#add_subdirectory(demo)


########### library target ###############

set(libsvm_LIB_SRCS 
     #SvmStructs.cpp
)

#add_library(libsvm SHARED ${libsvm_LIB_SRCS})

#target_link_libraries(libsvm   Qt5::Gui )

#set_target_properties(libsvm PROPERTIES VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION} )


########### install files ###############

#install(TARGETS libsvm  ${INSTALL_TARGETS_DEFAULT_ARGS})

#install(
#    FILES
#    SvmEnums.h
#    SvmStructs.h
#    svm_export.h
#    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
#)
