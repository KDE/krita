# include_directories(../../lib/ ../../lib/transformations)

set( libkritafilterslistdynamicprogram_LIB_SRCS
    kis_filters_list_dynamic_programs_editor.cc
    kis_filters_list_model.cc
    kis_filters_list_dynamic_program.cc
)

kde4_automoc(${libkritafilterslistdynamicprogram_LIB_SRCS})

kde4_add_ui_files(libkritafilterslistdynamicprogram_LIB_SRCS FiltersListDynamicProgramEditor.ui )

kde4_add_library(kritafilterslistdynamicprogram SHARED ${libkritafilterslistdynamicprogram_LIB_SRCS})

target_link_libraries(kritafilterslistdynamicprogram kritadynamicbrush)


install(TARGETS kritafilterslistdynamicprogram  DESTINATION ${LIB_INSTALL_DIR})

