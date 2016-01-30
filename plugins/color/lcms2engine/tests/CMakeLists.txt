add_definitions(-DFILES_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ../colorspaces/cmyk_u16
    ../colorspaces/cmyk_u8
    ../colorspaces/gray_u16
    ../colorspaces/gray_u16_no_alpha
    ../colorspaces/gray_u8
    ../colorspaces/gray_u8_no_alpha
    ../colorspaces/lab_u16
    ../colorspaces/rgb_u16
    ../colorspaces/rgb_u8
    ../colorspaces/xyz_u16
    ../colorprofiles
    ..
)
if(OPENEXR_FOUND)
    include_directories(${OPENEXR_INCLUDE_DIR})
endif()
include_directories( ${LCMS2_INCLUDE_DIR} )


if(MSVC OR (WIN32 AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel"))
  # avoid "cannot open file 'LIBC.lib'" error
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBC.LIB")
endif()

########### next target ###############

set(TestKoLcmsColorProfile_test_SRCS TestKoLcmsColorProfile.cpp )

kde4_add_unit_test(TestKoLcmsColorProfile TESTNAME libs-pigment-TestKoLcmsColorProfile  ${TestKoLcmsColorProfile_test_SRCS})

target_link_libraries(TestKoLcmsColorProfile kritawidgets kritapigment KF5::I18n  Qt5::Test ${LCMS2_LIBRARIES} kritalcmsengine )

########### next target ###############

set(TestKoColorSpaceRegistry_test_SRCS TestKoColorSpaceRegistry.cpp )

kde4_add_unit_test(TestKoColorSpaceRegistry TESTNAME libs-pigment-TestKoColorSpaceRegistry  ${TestKoColorSpaceRegistry_test_SRCS})

target_link_libraries(TestKoColorSpaceRegistry kritawidgets kritapigment ${LCMS2_LIBRARIES} KF5::I18n  Qt5::Test)

########### next target ###############

#set(TestKoCompositeOps_test_SRCS TestKoCompositeOps.cpp )

#kde4_add_unit_test(TestKoCompositeOps TESTNAME libs-pigment-TestKoCompositeOps ${TestKoCompositeOps_test_SRCS})

#target_link_libraries(TestKoCompositeOps  kritapigment Qt5::Test)


