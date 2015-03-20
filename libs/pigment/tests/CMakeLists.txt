set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

add_definitions(-DFILES_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")

include_directories( ${PIGMENT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/../colorspaces )

if(MSVC OR (WIN32 AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel"))
  # avoid "cannot open file 'LIBC.lib'" error
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBC.LIB")
endif()

########### next target ###############

set(TestKoColorSpaceRegistry_test_SRCS TestKoColorSpaceRegistry.cpp )

add_executable(TestKoColorSpaceRegistryNAME libs-pigment-TestKoColorSpaceRegistry ${TestKoColorSpaceRegistry_test_SRCS})
add_test(TestKoColorSpaceRegistry TestKoColorSpaceRegistry)
ecm_mark_as_test(TestKoColorSpaceRegistry)

target_link_libraries(TestKoColorSpaceRegistry   pigmentcms Qt5::Test)

########### next target ###############

set(TestColorConversion_test_SRCS TestColorConversion.cpp )

add_executable(TestColorConversionNAME libs-pigment-TestColorConversion  ${TestColorConversion_test_SRCS})
add_test(TestColorConversion TestColorConversion)
ecm_mark_as_test(TestColorConversion)

target_link_libraries(TestColorConversion   pigmentcms Qt5::Test)

########### next target ###############

set(TestKoColorSpaceAbstract_test_SRCS TestKoColorSpaceAbstract.cpp )

add_executable(TestKoColorSpaceAbstractNAME libs-pigment-TestKoColorSpaceAbstract  ${TestKoColorSpaceAbstract_test_SRCS})
add_test(TestKoColorSpaceAbstract TestKoColorSpaceAbstract)
ecm_mark_as_test(TestKoColorSpaceAbstract)

target_link_libraries(TestKoColorSpaceAbstract   pigmentcms Qt5::Test)

########### next target ###############

set(TestKoColorSpaceMaths_test_SRCS TestKoColorSpaceMaths.cpp)

add_executable(TestKoColorSpaceMathsNAME libs-pigment-TestKoColorSpaceMaths ${TestKoColorSpaceMaths_test_SRCS})
add_test(TestKoColorSpaceMaths TestKoColorSpaceMaths)
ecm_mark_as_test(TestKoColorSpaceMaths)

target_link_libraries(TestKoColorSpaceMaths   pigmentcms Qt5::Test)

########### next target ###############
set(CCSGraph_GRAPH CCSGraph.cpp)
add_executable(CCSGraph ${CCSGraph_GRAPH})
target_link_libraries(CCSGraph   pigmentcms )

########### next target ###############

set(TestColorConversionSystem_test_SRCS TestColorConversionSystem.cpp )

add_executable(TestColorConversionSystemNAME libs-pigment-TestColorConversionSystem ${TestColorConversionSystem_test_SRCS})
add_test(TestColorConversionSystem TestColorConversionSystem)
ecm_mark_as_test(TestColorConversionSystem)

target_link_libraries(TestColorConversionSystem   pigmentcms Qt5::Test)

########### next target ###############

set(TestKoColor_test_SRCS TestKoColor.cpp )

add_executable(TestKoColorNAME libs-pigment-TestKoColor ${TestKoColor_test_SRCS})
add_test(TestKoColor TestKoColor)
ecm_mark_as_test(TestKoColor)

target_link_libraries(TestKoColor   pigmentcms Qt5::Xml Qt5::Test)


########### next target ###############

set(TestKoIntegerMaths_test_SRCS TestKoIntegerMaths.cpp )
add_executable(TestKoIntegerMathsNAME libs-pigment-TestKoIntegerMaths ${TestKoIntegerMaths_test_SRCS})
add_test(TestKoIntegerMaths TestKoIntegerMaths)
ecm_mark_as_test(TestKoIntegerMaths)
target_link_libraries(TestKoIntegerMaths   pigmentcms Qt5::Test)

########### next target ###############

set(TestConvolutionOpImpl_test_SRCS TestConvolutionOpImpl.cpp )

add_executable(TestConvolutionOpImplNAME libs-pigment-TestConvolutionOpImpl ${TestConvolutionOpImpl_test_SRCS})
add_test(TestConvolutionOpImpl TestConvolutionOpImpl)
ecm_mark_as_test(TestConvolutionOpImpl)

target_link_libraries(TestConvolutionOpImpl   pigmentcms Qt5::Test)


########### next target ###############

set(TestKoRgbU8ColorSpaceTester_test_SRCS KoRgbU8ColorSpaceTester.cpp )

add_executable(TestKoRgbU8ColorSpaceTesterNAME libs-pigment-TestKoRgbU8ColorSpaceTester ${TestKoRgbU8ColorSpaceTester_test_SRCS})
add_test(TestKoRgbU8ColorSpaceTester TestKoRgbU8ColorSpaceTester)
ecm_mark_as_test(TestKoRgbU8ColorSpaceTester)

target_link_libraries(TestKoRgbU8ColorSpaceTester   pigmentcms Qt5::Test)

########### next target ###############

set(TestKoColorSpaceSanity_test_SRCS TestKoColorSpaceSanity.cpp )

add_executable(TestKoColorSpaceSanityNAME libs-pigment-TestKoColorSpaceSanity ${TestKoColorSpaceSanity_test_SRCS})
add_test(TestKoColorSpaceSanity TestKoColorSpaceSanity)
ecm_mark_as_test(TestKoColorSpaceSanity)

target_link_libraries(TestKoColorSpaceSanity   pigmentcms Qt5::Test)

########### next target ###############

set(TestFallBackColorTransformation_test_SRCS TestFallBackColorTransformation.cpp )

add_executable(TestFallBackColorTransformationNAME libs-pigment-TestFallBackColorTransformation ${TestFallBackColorTransformation_test_SRCS})
add_test(TestFallBackColorTransformation TestFallBackColorTransformation)
ecm_mark_as_test(TestFallBackColorTransformation)

target_link_libraries(TestFallBackColorTransformation   pigmentcms Qt5::Test)

########### next target ###############

set(TestKoChannelInfo_test_SRCS TestKoChannelInfo.cpp )

add_executable(TestKoChannelInfoNAME libs-pigment-TestKoChannelInfo ${TestKoChannelInfo_test_SRCS})
add_test(TestKoChannelInfo TestKoChannelInfo)
ecm_mark_as_test(TestKoChannelInfo)

target_link_libraries(TestKoChannelInfo   pigmentcms Qt5::Test)
