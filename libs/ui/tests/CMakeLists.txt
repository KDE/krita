kis_add_tests(
    kis_image_view_converter_test.cpp
    kis_shape_selection_test.cpp
    kis_coordinates_converter_test.cpp
    kis_grid_config_test.cpp
    kis_stabilized_events_sampler_test.cpp
    kis_shape_commands_test.cpp
    kis_stop_gradient_editor_test.cpp
    kis_multinode_property_test.cpp
    KisFrameSerializerTest.cpp
    KisFrameCacheStoreTest.cpp
    kis_animation_exporter_test.cpp
    kis_prescaled_projection_test.cpp
    kis_animation_importer_test.cpp
    KisSpinBoxSplineUnitConverterTest.cpp
    KisDocumentReplaceTest.cpp
    KisRssReaderTest.cpp
    kis_derived_resources_test.cpp
    kis_animation_frame_cache_test.cpp
    kis_shape_layer_test.cpp
    KisSafeDocumentLoaderTest.cpp
    KisSurfaceColorSpaceWrapperTest.cpp

    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "libs-ui-"
    )

kis_add_tests(
    kis_doc2_test.cpp
    kis_file_layer_test.cpp
    LINK_LIBRARIES kritaui kritatransformmaskstubs
    NAME_PREFIX "libs-ui-")

if (ENABLE_UPDATERS)
if (UNIX)
    kis_add_test( KisAppimageUpdaterTest.cpp
        TEST_NAME KisAppimageUpdaterTest
        LINK_LIBRARIES kritaui kritatestsdk
        NAME_PREFIX "libs-ui-")
endif()

kis_add_test( KisManualUpdaterTest.cpp ../../../sdk/tests/testutil.cpp MockMultiFeedRssModel.cpp
    TEST_NAME KisManualUpdaterTest
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "libs-ui-")
endif()


kis_add_test( KisMultiFeedRssModelTest.cpp ../../../sdk/tests/testutil.cpp MockNetworkAccessManager.cpp
    TEST_NAME KisMultiFeedRssModelTest
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "libs-ui-")


kis_add_test( kis_selection_decoration_test.cpp ../../../sdk/tests/stroke_testing_utils.cpp
    TEST_NAME KisSelectionDecorationTest
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "libs-ui-")

kis_add_test( kis_node_dummies_graph_test.cpp ../../../sdk/tests/testutil.cpp
    TEST_NAME KisNodeDummiesGraphTest
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "libs-ui-")

kis_add_test( kis_node_shapes_graph_test.cpp ../../../sdk/tests/testutil.cpp
    TEST_NAME KisNodeShapesGraphTest
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "libs-ui-")

kis_add_test( kis_model_index_converter_test.cpp ../../../sdk/tests/testutil.cpp
    TEST_NAME KisModelIndexConverterTest
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "libs-ui-")


kis_add_test( kis_categorized_list_model_test.cpp modeltest.cpp
    TEST_NAME KisCategorizedListModelTest
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "libs-ui-")

kis_add_test( kis_node_juggler_compressed_test.cpp  ../../../sdk/tests/testutil.cpp
    TEST_NAME KisNodeJugglerCompressedTest
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "libs-ui-")

kis_add_test(
    kis_input_manager_test.cpp ../../../sdk/tests/testutil.cpp
    TEST_NAME KisInputManagerTest
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "libs-ui-")

kis_add_test(
    kis_node_model_test.cpp modeltest.cpp
    TEST_NAME kis_node_model_test
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "libs-ui-")

kis_add_test( kis_shape_controller_test.cpp kis_dummies_facade_base_test.cpp
    TEST_NAME kis_shape_controller_test
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "libs-ui-")

kis_add_test( kis_dummies_facade_test.cpp kis_dummies_facade_base_test.cpp ../../../sdk/tests/testutil.cpp
    TEST_NAME kis_dummies_facade_test
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "libs-ui-")


##### Tests that currently fail and should be fixed #####

# Works under Linux but does not work under Windows
krita_add_broken_unit_test(
    kis_painting_assistants_decoration_test.cpp ../../../sdk/tests/testutil.cpp
    TEST_NAME KisPaintingAssistantsDecorationTest
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "libs-ui-"
    )

krita_add_broken_unit_test( kis_clipboard_test.cpp
    TEST_NAME KisClipboardTest
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "libs-ui-"
    )

krita_add_broken_unit_test( freehand_stroke_test.cpp  $<TARGET_PROPERTY:kritatestsdk,SOURCE_DIR>/stroke_testing_utils.cpp
    TEST_NAME FreehandStrokeTest
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "libs-ui-"
    )

krita_add_broken_unit_test( FreehandStrokeBenchmark.cpp  $<TARGET_PROPERTY:kritatestsdk,SOURCE_DIR>/stroke_testing_utils.cpp
    TEST_NAME FreehandStrokeBenchmark
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "libs-ui-"
    )

krita_add_broken_unit_test( KisPaintOnTransparencyMaskTest.cpp  $<TARGET_PROPERTY:kritatestsdk,SOURCE_DIR>/stroke_testing_utils.cpp
    TEST_NAME KisPaintOnTransparencyMaskTest
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "libs-ui-"
    )

krita_add_broken_unit_test( fill_processing_visitor_test.cpp  $<TARGET_PROPERTY:kritatestsdk,SOURCE_DIR>/stroke_testing_utils.cpp
    TEST_NAME FillProcessingVisitorTest
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "libs-ui-"
    )

krita_add_broken_unit_test( filter_stroke_test.cpp ../../../sdk/tests/stroke_testing_utils.cpp
    TEST_NAME FilterStrokeTest
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "libs-ui-"
    )

krita_add_broken_unit_test( kis_selection_manager_test.cpp
    TEST_NAME KisSelectionManagerTest
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "libs-ui-"
    )
#set_tests_properties(libs-ui-KisSelectionManagerTest PROPERTIES TIMEOUT 300)

krita_add_broken_unit_test( kis_node_manager_test.cpp
    TEST_NAME KisNodeManagerTest
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "libs-ui-"
    )

krita_add_broken_unit_test( kis_action_manager_test.cpp
    TEST_NAME KisActionManagerTest
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "libs-ui-"
    )

krita_add_broken_unit_test( kis_categories_mapper_test.cpp testing_categories_mapper.cpp
    TEST_NAME KisCategoriesMapperTest
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "libs-ui-"
    )


if (${INSTALL_BENCHMARKS})
    install(TARGETS FreehandStrokeBenchmark  ${INSTALL_TARGETS_DEFAULT_ARGS})

    install(FILES data/testing_200px_colorsmudge_default_dulling_old_sa.kpp
        data/testing_200px_colorsmudge_default_dulling_new_nsa.kpp
        data/testing_200px_colorsmudge_default_dulling_new_sa.kpp
        data/testing_800px_colorsmudge_default_dulling_new_sa.kpp
        data/testing_200px_colorsmudge_default_dulling_old_nsa.kpp
        data/testing_200px_colorsmudge_default_smearing_new_nsa.kpp
        data/testing_200px_colorsmudge_default_smearing_new_sa.kpp
        data/testing_200px_colorsmudge_default_smearing_old_nsa.kpp
        data/testing_200px_colorsmudge_default_smearing_old_sa.kpp
        data/testing_200px_colorsmudge_lightness_smearing_new_nsa_maskmode.kpp
        data/testing_200px_colorsmudge_lightness_smearing_new_nsa_nopt.kpp
        data/testing_200px_colorsmudge_lightness_smearing_new_nsa_ptoverlay.kpp
        data/testing_200px_colorsmudge_lightness_smearing_new_nsa_ptoverwrite.kpp
        data/testing_1000px_auto_default.kpp
        data/testing_1000px_auto_gaussian.kpp
        data/testing_1000px_auto_gaussian_rect.kpp
        data/testing_1000px_auto_rectangular.kpp
        data/testing_1000px_auto_soft.kpp
        data/testing_1000px_auto_soft_rect.kpp
        data/testing_1000px_stamp_450_rotated.kpp
        data/3_texture.png
        data/DA_RGBA\ bluegreen_small1.png
        DESTINATION ${CMAKE_INSTALL_BINDIR}/data)
endif()
