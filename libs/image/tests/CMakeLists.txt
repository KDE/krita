# cmake in some versions for some not yet known reasons fails to run automoc
# on random targets (changing target names already has an effect)
# As temporary workaround skipping build of tests on these versions for now
# See https://mail.kde.org/pipermail/kde-buildsystem/2015-June/010819.html
# extend range of affected cmake versions as needed
if(NOT ${CMAKE_VERSION} VERSION_LESS 3.1.3 AND
   NOT ${CMAKE_VERSION} VERSION_GREATER 3.2.3)
    message(WARNING "Skipping krita/image/tests, CMake in at least versions 3.1.3 - 3.2.3 seems to have a problem with automoc.  \n(FRIENDLY REMINDER: PLEASE DON'T BREAK THE TESTS!)")
    set (HAVE_FAILING_CMAKE TRUE)
else()
    set (HAVE_FAILING_CMAKE FALSE)
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}/libs/image/metadata
    ${CMAKE_BINARY_DIR}/libs/image/
    ${CMAKE_SOURCE_DIR}/libs/image/
    ${CMAKE_SOURCE_DIR}/libs/image/brushengine
    ${CMAKE_SOURCE_DIR}/libs/image/tiles3
    ${CMAKE_SOURCE_DIR}/libs/image/tiles3/swap
    ${CMAKE_SOURCE_DIR}/sdk/tests
)

include_Directories(SYSTEM
    ${EIGEN3_INCLUDE_DIR}
)

if(HAVE_VC)
  include_directories(${Vc_INCLUDE_DIR})
endif()

include(ECMAddTests)
include(KritaAddBrokenUnitTest)

macro_add_unittest_definitions()

set(KisRandomGeneratorDemoSources kis_random_generator_demo.cpp kimageframe.cpp)
ki18n_wrap_ui(KisRandomGeneratorDemoSources kis_random_generator_demo.ui)
add_executable(KisRandomGeneratorDemo ${KisRandomGeneratorDemoSources})
target_link_libraries(KisRandomGeneratorDemo kritaimage)
ecm_mark_as_test(KisRandomGeneratorDemo)

ecm_add_tests(
    kis_base_node_test.cpp
    kis_fast_math_test.cpp
    kis_node_test.cpp
    kis_node_facade_test.cpp
    kis_fixed_paint_device_test.cpp
    kis_layer_test.cpp
    kis_effect_mask_test.cpp
    kis_iterator_test.cpp
    kis_painter_test.cpp
    kis_selection_test.cpp
    kis_count_visitor_test.cpp
    kis_projection_test.cpp
    kis_properties_configuration_test.cpp
    kis_transaction_test.cpp
    kis_pixel_selection_test.cpp
    kis_group_layer_test.cpp
    kis_paint_layer_test.cpp
    kis_adjustment_layer_test.cpp
    kis_annotation_test.cpp
    kis_change_profile_visitor_test.cpp
    kis_clone_layer_test.cpp
    kis_colorspace_convert_visitor_test.cpp
    kis_convolution_painter_test.cpp
    kis_crop_processing_visitor_test.cpp
    kis_processing_applicator_test.cpp
    kis_datamanager_test.cpp
    kis_fill_painter_test.cpp
    kis_filter_configuration_test.cpp
    kis_filter_test.cpp
    kis_filter_processing_information_test.cpp
    kis_filter_registry_test.cpp
    kis_filter_strategy_test.cpp
    kis_gradient_painter_test.cpp
    kis_image_commands_test.cpp
    kis_image_test.cpp
    kis_image_signal_router_test.cpp
    kis_iterators_ng_test.cpp
    kis_iterator_benchmark.cpp
    kis_updater_context_test.cpp
    kis_simple_update_queue_test.cpp
    kis_stroke_test.cpp
    kis_simple_stroke_strategy_test.cpp
    kis_stroke_strategy_undo_command_based_test.cpp
    kis_strokes_queue_test.cpp
    kis_mask_test.cpp
    kis_math_toolbox_test.cpp
    kis_name_server_test.cpp
    kis_node_commands_test.cpp
    kis_node_graph_listener_test.cpp
    kis_node_visitor_test.cpp
    kis_paint_information_test.cpp
    kis_distance_information_test.cpp
    kis_paintop_test.cpp
    kis_pattern_test.cpp
    kis_selection_mask_test.cpp
    kis_shared_ptr_test.cpp
    kis_bsplines_test.cpp
    kis_warp_transform_worker_test.cpp
    kis_liquify_transform_worker_test.cpp
    kis_transparency_mask_test.cpp
    kis_types_test.cpp
    kis_vec_test.cpp
    kis_filter_config_widget_test.cpp
    kis_mask_generator_test.cpp
    kis_cubic_curve_test.cpp
    kis_fixed_point_maths_test.cpp
    kis_node_query_path_test.cpp
    kis_filter_weights_buffer_test.cpp
    kis_filter_weights_applicator_test.cpp
    kis_fill_interval_test.cpp
    kis_fill_interval_map_test.cpp
    kis_scanline_fill_test.cpp
    kis_psd_layer_style_test.cpp
    kis_layer_style_projection_plane_test.cpp
    kis_lod_capable_layer_offset_test.cpp
    kis_algebra_2d_test.cpp
    kis_marker_painter_test.cpp
    kis_lazy_brush_test.cpp
    kis_colorize_mask_test.cpp
    kis_mask_similarity_test.cpp
    KisMaskGeneratorBenchmark.cpp
    kis_time_range_test.cpp

    NAME_PREFIX "krita-image-"
    LINK_LIBRARIES kritaimage Qt5::Test)


ecm_add_test(kis_layer_style_filter_environment_test.cpp
    TEST_NAME kritaimage-layer_style_filter_environment_test
    LINK_LIBRARIES ${KDE4_KDEUI_LIBS} kritaimage Qt5::Test)

ecm_add_test(kis_asl_parser_test.cpp
    TEST_NAME kritalibpsd-asl_parser_test
    LINK_LIBRARIES kritapsd kritapigment kritawidgetutils kritacommand Qt5::Xml Qt5::Test)

ecm_add_test(KisPerStrokeRandomSourceTest.cpp
    TEST_NAME KisPerStrokeRandomSourceTest
    LINK_LIBRARIES kritaimage Qt5::Test)

ecm_add_test(KisWatershedWorkerTest.cpp
    TEST_NAME KisWatershedWorkerTest
    LINK_LIBRARIES kritaimage Qt5::Test)


# ecm_add_test(kis_dom_utils_test.cpp
#    TEST_NAME krita-image-DomUtils-Test
#    LINK_LIBRARIES kritaimage Qt5::Test)

# kisdoc dep
#    kis_transform_worker_test.cpp
#    TEST_NAME krita-image-KisTransformWorkerTest
#LINK_LIBRARIES kritaimage Qt5::Test)


# kisdoc
#    kis_perspective_transform_worker_test.cpp
#    TEST_NAME krita-image-KisPerspectiveTransformWorkerTest
#LINK_LIBRARIES kritaimage Qt5::Test)

#    kis_cs_conversion_test.cpp
#    TEST_NAME krita-image-KisCsConversionTest
#   LINK_LIBRARIES kritaimage Qt5::Test)

# kisdoc
#    kis_processings_test.cpp
#    TEST_NAME krita-image-KisProcessingsTest
#LINK_LIBRARIES kritaimage Qt5::Test)

# image/tests cannot use stuff that needs kisdocument
#    kis_projection_leaf_test.cpp
#    TEST_NAME kritaimage-projection_leaf_test
#    LINK_LIBRARIES kritaimage Qt5::Test)



if (NOT HAVE_FAILING_CMAKE)
  krita_add_broken_unit_test(kis_paint_device_test.cpp
      TEST_NAME krita-image-KisPaintDeviceTest
      LINK_LIBRARIES kritaimage kritaodf Qt5::Test)
else()
  message(WARNING "Skipping KisPaintDeviceTest!!!!!!!!!!!!!!")
endif()

if (NOT HAVE_FAILING_CMAKE)
  krita_add_broken_unit_test(kis_filter_mask_test.cpp
      TEST_NAME krita-image-KisFilterMaskTest
      LINK_LIBRARIES kritaimage Qt5::Test)
else()
  message(WARNING "Skipping KisFilterMaskTest!!!!!!!!!!!!!!")
endif()

krita_add_broken_unit_test(kis_transform_mask_test.cpp
    TEST_NAME krita-image-KisTransformMaskTest
    LINK_LIBRARIES kritaimage Qt5::Test)

krita_add_broken_unit_test(kis_histogram_test.cpp
    TEST_NAME krita-image-KisHistogramTest
    LINK_LIBRARIES kritaimage Qt5::Test)

krita_add_broken_unit_test(kis_walkers_test.cpp
    TEST_NAME krita-image-KisWalkersTest
    LINK_LIBRARIES kritaimage Qt5::Test)

#krita_add_broken_unit_test(kis_async_merger_test.cpp
#    TEST_NAME krita-image-KisAsyncMergerTest
#    LINK_LIBRARIES kritaimage Qt5::Test)

krita_add_broken_unit_test(kis_update_scheduler_test.cpp
    TEST_NAME krita-image-KisUpdateSchedulerTest
    LINK_LIBRARIES kritaimage Qt5::Test)

krita_add_broken_unit_test(kis_queues_progress_updater_test.cpp
    TEST_NAME krita-image-KisQueuesProgressUpdaterTest
    LINK_LIBRARIES kritaimage Qt5::Test)

krita_add_broken_unit_test(kis_cage_transform_worker_test.cpp
    TEST_NAME krita-image-KisCageTransformWorkerTest
    LINK_LIBRARIES kritaimage Qt5::Test)

krita_add_broken_unit_test(kis_meta_data_test.cpp
    TEST_NAME krita-image-KisMetaDataTest
    LINK_LIBRARIES kritaimage Qt5::Test)

krita_add_broken_unit_test(kis_random_generator_test.cpp
    TEST_NAME krita-image-KisRandomGeneratorTest
    LINK_LIBRARIES kritaimage Qt5::Test)

krita_add_broken_unit_test(kis_keyframing_test.cpp
    TEST_NAME krita-image-Keyframing-Test
    LINK_LIBRARIES kritaimage Qt5::Test)

krita_add_broken_unit_test(kis_image_animation_interface_test.cpp
    TEST_NAME krita-image-ImageAnimationInterface-Test
    LINK_LIBRARIES ${KDE4_KDEUI_LIBS} kritaimage Qt5::Test)

krita_add_broken_unit_test(kis_onion_skin_compositor_test.cpp
    TEST_NAME krita-image-OnionSkinCompositor-Test
    LINK_LIBRARIES ${KDE4_KDEUI_LIBS} kritaimage Qt5::Test)

krita_add_broken_unit_test(kis_layer_styles_test.cpp
    TEST_NAME krita-image-LayerStylesTest
    LINK_LIBRARIES kritaimage Qt5::Test)
