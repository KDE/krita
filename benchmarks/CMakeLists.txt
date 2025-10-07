########### next target ###############

set(kis_datamanager_benchmark_SRCS kis_datamanager_benchmark.cpp)
set(kis_hiterator_benchmark_SRCS kis_hline_iterator_benchmark.cpp)
set(kis_viterator_benchmark_SRCS kis_vline_iterator_benchmark.cpp)
set(kis_random_iterator_benchmark_SRCS kis_random_iterator_benchmark.cpp)
set(kis_projection_benchmark_SRCS kis_projection_benchmark.cpp)
set(kis_bcontrast_benchmark_SRCS kis_bcontrast_benchmark.cpp)
set(kis_blur_benchmark_SRCS kis_blur_benchmark.cpp)
set(kis_level_filter_benchmark_SRCS kis_level_filter_benchmark.cpp)
set(kis_painter_benchmark_SRCS kis_painter_benchmark.cpp)
set(kis_stroke_benchmark_SRCS kis_stroke_benchmark.cpp)
set(kis_fast_math_benchmark_SRCS kis_fast_math_benchmark.cpp)
set(kis_floodfill_benchmark_SRCS kis_floodfill_benchmark.cpp)
set(kis_gradient_benchmark_SRCS kis_gradient_benchmark.cpp)
set(kis_mask_generator_benchmark_SRCS kis_mask_generator_benchmark.cpp)
set(kis_low_memory_benchmark_SRCS kis_low_memory_benchmark.cpp)
set(KisAnimationRenderingBenchmark_SRCS KisAnimationRenderingBenchmark.cpp)
set(kis_filter_selections_benchmark_SRCS kis_filter_selections_benchmark.cpp)
set(kis_thumbnail_benchmark_SRCS kis_thumbnail_benchmark.cpp)
set(KisLiquifyTransformWorkerBenchmark_SRCS KisLiquifyTransformWorkerBenchmark.cpp)
set(KisFourPointInterpolatorBenchmark_SRCS KisFourPointInterpolatorBenchmark.cpp)


krita_add_benchmark(KisDatamanagerBenchmark TESTNAME krita-benchmarks-KisDataManager ${kis_datamanager_benchmark_SRCS})
krita_add_benchmark(KisHLineIteratorBenchmark TESTNAME krita-benchmarks-KisHLineIterator ${kis_hiterator_benchmark_SRCS})
krita_add_benchmark(KisVLineIteratorBenchmark TESTNAME krita-benchmarks-KisVLineIterator ${kis_viterator_benchmark_SRCS})
krita_add_benchmark(KisRandomIteratorBenchmark TESTNAME krita-benchmarks-KisRandomIterator ${kis_random_iterator_benchmark_SRCS})
krita_add_benchmark(KisProjectionBenchmark TESTNAME krita-benchmarks-KisProjectionBenchmark ${kis_projection_benchmark_SRCS})
krita_add_benchmark(KisBContrastBenchmark TESTNAME krita-benchmarks-KisBContrastBenchmark ${kis_bcontrast_benchmark_SRCS})
krita_add_benchmark(KisBlurBenchmark TESTNAME krita-benchmarks-KisBlurBenchmark ${kis_blur_benchmark_SRCS})
krita_add_benchmark(KisLevelFilterBenchmark TESTNAME krita-benchmarks-KisLevelFilterBenchmark ${kis_level_filter_benchmark_SRCS})
krita_add_benchmark(KisPainterBenchmark TESTNAME krita-benchmarks-KisPainterBenchmark ${kis_painter_benchmark_SRCS})
krita_add_benchmark(KisStrokeBenchmark TESTNAME krita-benchmarks-KisStrokeBenchmark ${kis_stroke_benchmark_SRCS})
krita_add_benchmark(KisFastMathBenchmark TESTNAME krita-benchmarks-KisFastMath ${kis_fast_math_benchmark_SRCS})
krita_add_benchmark(KisFloodfillBenchmark TESTNAME krita-benchmarks-KisFloodFill ${kis_floodfill_benchmark_SRCS})
krita_add_benchmark(KisGradientBenchmark TESTNAME krita-benchmarks-KisGradientFill ${kis_gradient_benchmark_SRCS})
krita_add_benchmark(KisMaskGeneratorBenchmark TESTNAME krita-benchmarks-KisMaskGenerator ${kis_mask_generator_benchmark_SRCS})
krita_add_benchmark(KisLowMemoryBenchmark TESTNAME krita-benchmarks-KisLowMemory ${kis_low_memory_benchmark_SRCS})
krita_add_benchmark(KisAnimationRenderingBenchmark TESTNAME krita-benchmarks-KisAnimationRenderingBenchmark ${KisAnimationRenderingBenchmark_SRCS})
krita_add_benchmark(KisFilterSelectionsBenchmark TESTNAME krita-image-KisFilterSelectionsBenchmark ${kis_filter_selections_benchmark_SRCS})
krita_add_benchmark(KisThumbnailBenchmark TESTNAME krita-benchmarks-KisThumbnail ${kis_thumbnail_benchmark_SRCS})
krita_add_benchmark(KisLiquifyTransformWorkerBenchmark TESTNAME krita-benchmarks-KisLiquifyTransformWorker ${KisLiquifyTransformWorkerBenchmark_SRCS})
krita_add_benchmark(KisFourPointInterpolatorBenchmark TESTNAME krita-benchmarks-KisFourPointInterpolator ${KisFourPointInterpolatorBenchmark_SRCS})


include_directories(${QUAZIP_INCLUDE_DIRS})


target_link_libraries(KisDatamanagerBenchmark  kritaimage  kritatestsdk)
target_link_libraries(KisHLineIteratorBenchmark  kritaimage  kritatestsdk)
target_link_libraries(KisVLineIteratorBenchmark  kritaimage  kritatestsdk)
target_link_libraries(KisRandomIteratorBenchmark  kritaimage  kritatestsdk)
target_link_libraries(KisProjectionBenchmark  kritaimage  kritaui kritatestsdk)
target_link_libraries(KisBContrastBenchmark  kritaimage  kritatestsdk)
target_link_libraries(KisBlurBenchmark  kritaimage  kritatestsdk)
target_link_libraries(KisLevelFilterBenchmark kritaimage  kritatestsdk)
target_link_libraries(KisPainterBenchmark  kritaimage  kritatestsdk)
target_link_libraries(KisStrokeBenchmark  kritaimage  kritatestsdk)
target_link_libraries(KisFastMathBenchmark  kritaimage  kritatestsdk)
target_link_libraries(KisFloodfillBenchmark  kritaimage  kritatestsdk)
target_link_libraries(KisGradientBenchmark  kritaimage  kritatestsdk)
target_link_libraries(KisLowMemoryBenchmark  kritaimage  kritatestsdk)
target_link_libraries(KisAnimationRenderingBenchmark  kritaimage kritaui  kritatestsdk)
target_link_libraries(KisFilterSelectionsBenchmark   kritaimage  kritatestsdk)
target_link_libraries(KisLiquifyTransformWorkerBenchmark   kritaimage  kritatestsdk ${QUAZIP_LIBRARIES})
target_link_libraries(KisFourPointInterpolatorBenchmark   kritaimage  kritatestsdk)


ko_compile_for_all_implementations_no_scalar(__per_arch_composition_objects kis_composition_benchmark.cpp)
message("Following objects are generated for the composition benchmark")
foreach(_obj IN LISTS __per_arch_composition_objects)
    string(REPLACE "\.cpp" "" _target ${_obj})
    string(REPLACE "\.cpp" "\.h" _fake_header ${_obj})
    get_filename_component(_target ${_target} NAME)
    message("    * ${_target} <- ${_obj}")
    krita_add_benchmark(${_target} TESTNAME "krita-benchmarks-${_target}" ${_obj})

    # Automoc goes crazy if the source file is located in the build
    # directory and does not have a header with exactly the same
    # base name. It just skips generation for such a source, since
    # it doesn't include Q_OBJECT macro directly.
    #
    # So here we just generate a fake header which makes
    # automoc happy
    #
    # `configure_file` is used to create a semantic dependency
    # between the two files

    configure_file(kis_composition_benchmark.h ${_fake_header} COPYONLY)

    target_link_libraries(${_target} kritaimage kritatestsdk kritamultiarch)
endforeach()

target_link_libraries(KisMaskGeneratorBenchmark  kritaimage  kritatestsdk)
target_link_libraries(KisThumbnailBenchmark  kritaimage  kritatestsdk)
