project(kritapigment)

# we have to repeat platform specifics from top-level
if (WIN32)
    include_directories(${CMAKE_SOURCE_DIR}/winquirks)
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DNOMINMAX)
    set(WIN32_PLATFORM_NET_LIBS ws2_32.lib netapi32.lib)
endif ()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/resources
    ${CMAKE_CURRENT_SOURCE_DIR}/compositeops)

set(FILE_OPENEXR_SOURCES)
set(LINK_OPENEXR_LIB)
if(OpenEXR_FOUND)
    include_directories(SYSTEM ${OPENEXR_INCLUDE_DIRS})
    set(LINK_OPENEXR_LIB ${OPENEXR_LIBRARIES})
    add_definitions(${OPENEXR_DEFINITIONS})
endif()

set(LINK_VC_LIB)

if(HAVE_VC)
    include_directories(SYSTEM ${Vc_INCLUDE_DIR})
    set(LINK_VC_LIB ${Vc_LIBRARIES})
    ko_compile_for_all_implementations_no_scalar(__per_arch_factory_objs compositeops/KoOptimizedCompositeOpFactoryPerArch.cpp)
    ko_compile_for_all_implementations(__per_arch_alpha_applicator_factory_objs KoAlphaMaskApplicatorFactoryImpl.cpp)
    ko_compile_for_all_implementations(__per_arch_rgb_scaler_factory_objs KoOptimizedRgbPixelDataScalerU8ToU16FactoryImpl.cpp)

    message("Following objects are generated from the per-arch lib")
    message("${__per_arch_factory_objs}")
else()
    set(__per_arch_alpha_applicator_factory_objs KoAlphaMaskApplicatorFactoryImpl.cpp)
endif()

add_subdirectory(tests)
add_subdirectory(benchmarks)

set(kritapigment_SRCS
    DebugPigment.cpp
    KoBasicHistogramProducers.cpp
    KoAlphaMaskApplicatorBase.cpp
    KoOptimizedRgbPixelDataScalerU8ToU16Base.cpp
    KoOptimizedRgbPixelDataScalerU8ToU16Factory.cpp
    KoColor.cpp
    KoColorDisplayRendererInterface.cpp
    KoColorConversionAlphaTransformation.cpp
    KoColorConversionCache.cpp
    KoColorConversions.cpp
    KoColorConversionSystem.cpp
    KoColorConversionTransformation.cpp
    KoColorProofingConversionTransformation.cpp
    KoColorConversionTransformationFactory.cpp
    KoColorModelStandardIds.cpp
    KoColorProfile.cpp
    KoColorSpace.cpp
    KoColorSpaceEngine.cpp
    KoColorSpaceFactory.cpp
    KoColorSpaceMaths.cpp
    KoCmykColorSpaceMaths.cpp
    KoLabColorSpaceMaths.cpp
    KoColorSpaceRegistry.cpp
    KoColorProfileStorage.cpp
    KoColorTransformation.cpp
    KoColorTransformationFactory.cpp
    KoColorTransformationFactoryRegistry.cpp
    KoCompositeColorTransformation.cpp
    KoCompositeOp.cpp
    KoCompositeOpRegistry.cpp
    KoCopyColorConversionTransformation.cpp
    KoFallBackColorTransformation.cpp
    KoHistogramProducer.cpp
    KoMultipleColorConversionTransformation.cpp
    KoUniqueNumberForIdServer.cpp
    colorspaces/KoAlphaColorSpace.cpp
    colorspaces/KoLabColorSpace.cpp
    colorspaces/KoRgbU16ColorSpace.cpp
    colorspaces/KoRgbU8ColorSpace.cpp
    colorspaces/KoSimpleColorSpaceEngine.cpp
    compositeops/KoOptimizedCompositeOpFactory.cpp
    compositeops/KoOptimizedCompositeOpFactoryPerArch_Scalar.cpp
    compositeops/KoAlphaDarkenParamsWrapper.cpp
    ${__per_arch_factory_objs}
    ${__per_arch_alpha_applicator_factory_objs}
    ${__per_arch_rgb_scaler_factory_objs}
    KoAlphaMaskApplicatorFactory.cpp
    colorprofiles/KoDummyColorProfile.cpp
    resources/KoAbstractGradient.cpp
    resources/KoColorSet.cpp
    resources/KisSwatch.cpp
    resources/KisSwatchGroup.cpp
    resources/KoPattern.cpp
    resources/KoStopGradient.cpp
    resources/KoSegmentGradient.cpp
    resources/KoCachedGradient.h
    resources/KisGradientConversion.cpp
)

set (EXTRA_LIBRARIES ${LINK_OPENEXR_LIB} ${LINK_VC_LIB})

if(MSVC OR (WIN32 AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel"))
  # avoid "cannot open file 'LIBC.lib'" error
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBC.LIB")
endif()

add_library(kritapigment SHARED ${kritapigment_SRCS})
generate_export_header(kritapigment)

target_include_directories( kritapigment
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/resources>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/compositeops>
)

target_link_libraries( kritapigment
    PUBLIC
        kritaplugin
        kritastore
        kritaglobal
        kritaresources
        ${EXTRA_LIBRARIES}
        KF5::I18n 
        KF5::ConfigCore
        Qt5::Core
        Qt5::Gui
        Qt5::Xml
        ${WIN32_PLATFORM_NET_LIBS}
)

set_target_properties(kritapigment PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

install(TARGETS kritapigment  ${INSTALL_TARGETS_DEFAULT_ARGS})
