include_directories(transformations )

set( libkritafilterslistdynamicprogram_LIB_SRCS
    kis_filters_list_model.cc
    kis_filters_list_dynamic_program.cc
    kis_filters_list_dynamic_program_editor.cc
    transformations/kis_dynamic_transformation.cc
    transformations/kis_dynamic_transformations_factory.cc
    transformations/kis_rotation_transformation.cc
    transformations/kis_size_transformation.cc
    transformations/kis_darken_transformation.cc
)

kde4_add_ui_files(libkritafilterslistdynamicprogram_LIB_SRCS 
    transformations/SizeTransformationEditor.ui
    transformations/RotationTransformationEditor.ui
    FiltersListDynamicProgramEditor.ui  )

kde4_add_library(kritafilterslistdynamicprogram SHARED ${libkritafilterslistdynamicprogram_LIB_SRCS})

target_link_libraries(kritafilterslistdynamicprogram kritadynamicbrush)

set_target_properties(kritafilterslistdynamicprogram PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )

install(TARGETS kritafilterslistdynamicprogram  DESTINATION ${LIB_INSTALL_DIR})

