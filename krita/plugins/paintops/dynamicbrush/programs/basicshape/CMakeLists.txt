
set( libkritabasicdynamicshapeprogram_LIB_SRCS
    kis_basic_dynamic_program.cc
    kis_basic_dynamic_program_editor.cc
)

kde4_add_ui_files(libkritabasicdynamicshapeprogram_LIB_SRCS BasicDynamicProgramEditor.ui  )

kde4_add_library(kritabasicdynamicshapeprogram SHARED ${libkritabasicdynamicshapeprogram_LIB_SRCS})

target_link_libraries(kritabasicdynamicshapeprogram kritadynamicbrush)

set_target_properties(kritabasicdynamicshapeprogram PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )

install(TARGETS kritabasicdynamicshapeprogram  DESTINATION ${LIB_INSTALL_DIR})

