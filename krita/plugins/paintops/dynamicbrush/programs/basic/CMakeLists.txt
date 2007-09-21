
set( libkritabasicdynamicprogram_LIB_SRCS
    kis_basic_dynamic_program.cc
    kis_basic_dynamic_program_editor.cc
)

kde4_add_ui_files(libkritabasicdynamicprogram_LIB_SRCS BasicDynamicProgramEditor.ui  )

kde4_add_library(kritabasicdynamicprogram SHARED ${libkritabasicdynamicprogram_LIB_SRCS})

target_link_libraries(kritabasicdynamicprogram kritadynamicbrush)

set_target_properties(kritabasicdynamicprogram PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )

install(TARGETS kritabasicdynamicprogram  DESTINATION ${LIB_INSTALL_DIR})

