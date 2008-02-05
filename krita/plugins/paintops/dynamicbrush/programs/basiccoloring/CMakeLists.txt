
set( libkritabasicdynamiccoloringprogram_LIB_SRCS
    kis_basic_dynamic_coloring_program.cc
    kis_basic_dynamic_coloring_program_editor.cc
)

kde4_add_ui_files(libkritabasicdynamiccoloringprogram_LIB_SRCS BasicDynamicColoringProgramEditor.ui  )

kde4_add_library(kritabasicdynamiccoloringprogram SHARED ${libkritabasicdynamiccoloringprogram_LIB_SRCS})

target_link_libraries(kritabasicdynamiccoloringprogram kritadynamicbrush)

set_target_properties(kritabasicdynamiccoloringprogram PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )

install(TARGETS kritabasicdynamiccoloringprogram  DESTINATION ${LIB_INSTALL_DIR})

