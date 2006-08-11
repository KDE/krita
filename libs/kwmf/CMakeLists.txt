

include_directories( ${KOFFICECORE_INCLUDES} )


########### next target ###############

set(kwmf_LIB_SRCS kwmf.cc qwmf.cc )

kde4_automoc(${kwmf_LIB_SRCS})

kde4_add_library(kwmf SHARED ${kwmf_LIB_SRCS})

target_link_libraries(kwmf  ${KDE4_KDECORE_LIBS} )

set_target_properties(kwmf PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kwmf  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(kowmf_LIB_SRCS 
   kowmfreadprivate.cc 
   kowmfstack.cc 
   kowmfread.cc 
   kowmfwrite.cc 
   kowmfpaint.cc )

kde4_automoc(${kowmf_LIB_SRCS})

kde4_add_library(kowmf SHARED ${kowmf_LIB_SRCS})

target_link_libraries(kowmf  ${KDE4_KDECORE_LIBS} )

set_target_properties(kowmf PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kowmf  DESTINATION ${LIB_INSTALL_DIR})

