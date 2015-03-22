set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

add_definitions(-DFILES_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")

include_directories( ${PIGMENT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/../colorspaces )

if(MSVC OR (WIN32 AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel"))
  # avoid "cannot open file 'LIBC.lib'" error
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBC.LIB")
endif()

########### next target ###############

set(TestKoColorSpaceRegistry_test_SRCS TestKoColorSpaceRegistry.cpp )

kde4_add_unit_test(TestKoColorSpaceRegistry TESTNAME libs-pigment-TestKoColorSpaceRegistry ${TestKoColorSpaceRegistry_test_SRCS})

target_link_libraries(TestKoColorSpaceRegistry  pigmentcms Qt5::Test)

########### next target ###############

set(TestColorConversion_test_SRCS TestColorConversion.cpp )

kde4_add_unit_test(TestColorConversion TESTNAME libs-pigment-TestColorConversion  ${TestColorConversion_test_SRCS})

target_link_libraries(TestColorConversion  pigmentcms Qt5::Test)

########### next target ###############

set(TestKoColorSpaceAbstract_test_SRCS TestKoColorSpaceAbstract.cpp )

kde4_add_unit_test(TestKoColorSpaceAbstract TESTNAME libs-pigment-TestKoColorSpaceAbstract  ${TestKoColorSpaceAbstract_test_SRCS})

target_link_libraries(TestKoColorSpaceAbstract  pigmentcms Qt5::Test)

########### next target ###############

set(TestKoColorSpaceMaths_test_SRCS TestKoColorSpaceMaths.cpp)

kde4_add_unit_test(TestKoColorSpaceMaths TESTNAME libs-pigment-TestKoColorSpaceMaths ${TestKoColorSpaceMaths_test_SRCS})

target_link_libraries(TestKoColorSpaceMaths  pigmentcms Qt5::Test)

########### next target ###############
set(CCSGraph_GRAPH CCSGraph.cpp)
kde4_add_executable(CCSGraph ${CCSGraph_GRAPH})
target_link_libraries(CCSGraph  pigmentcms )

########### next target ###############

set(TestColorConversionSystem_test_SRCS TestColorConversionSystem.cpp )

kde4_add_unit_test(TestColorConversionSystem TESTNAME libs-pigment-TestColorConversionSystem ${TestColorConversionSystem_test_SRCS})

target_link_libraries(TestColorConversionSystem  pigmentcms Qt5::Test)

########### next target ###############

set(TestKoColor_test_SRCS TestKoColor.cpp )

kde4_add_unit_test(TestKoColor TESTNAME libs-pigment-TestKoColor ${TestKoColor_test_SRCS})

target_link_libraries(TestKoColor  pigmentcms ${QT_QTXML_LIBRARY} Qt5::Test)


########### next target ###############

set(TestKoIntegerMaths_test_SRCS TestKoIntegerMaths.cpp )
kde4_add_unit_test(TestKoIntegerMaths TESTNAME libs-pigment-TestKoIntegerMaths ${TestKoIntegerMaths_test_SRCS})
target_link_libraries(TestKoIntegerMaths  pigmentcms Qt5::Test)

########### next target ###############

set(TestConvolutionOpImpl_test_SRCS TestConvolutionOpImpl.cpp )

kde4_add_unit_test(TestConvolutionOpImpl TESTNAME libs-pigment-TestConvolutionOpImpl ${TestConvolutionOpImpl_test_SRCS})

target_link_libraries(TestConvolutionOpImpl  pigmentcms Qt5::Test)


########### next target ###############

set(TestKoRgbU8ColorSpaceTester_test_SRCS KoRgbU8ColorSpaceTester.cpp )

kde4_add_unit_test(TestKoRgbU8ColorSpaceTester TESTNAME libs-pigment-TestKoRgbU8ColorSpaceTester ${TestKoRgbU8ColorSpaceTester_test_SRCS})

target_link_libraries(TestKoRgbU8ColorSpaceTester  pigmentcms Qt5::Test)

########### next target ###############

set(TestKoColorSpaceSanity_test_SRCS TestKoColorSpaceSanity.cpp )

kde4_add_unit_test(TestKoColorSpaceSanity TESTNAME libs-pigment-TestKoColorSpaceSanity ${TestKoColorSpaceSanity_test_SRCS})

target_link_libraries(TestKoColorSpaceSanity  pigmentcms Qt5::Test)

########### next target ###############

set(TestFallBackColorTransformation_test_SRCS TestFallBackColorTransformation.cpp )

kde4_add_unit_test(TestFallBackColorTransformation TESTNAME libs-pigment-TestFallBackColorTransformation ${TestFallBackColorTransformation_test_SRCS})

target_link_libraries(TestFallBackColorTransformation  pigmentcms Qt5::Test)

########### next target ###############

set(TestKoChannelInfo_test_SRCS TestKoChannelInfo.cpp )

kde4_add_unit_test(TestKoChannelInfo TESTNAME libs-pigment-TestKoChannelInfo ${TestKoChannelInfo_test_SRCS})

target_link_libraries(TestKoChannelInfo  pigmentcms Qt5::Test)
