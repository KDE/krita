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
endif(OPENEXR_FOUND)
include_directories( ${PIGMENT_INCLUDES} ${LCMS2_INCLUDE_DIR} )


if(MSVC)
  # avoid "cannot open file 'LIBC.lib'" error
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBC.LIB")
endif(MSVC)

########### next target ###############

set(TestKoLcmsColorProfile_test_SRCS TestKoLcmsColorProfile.cpp )

kde4_add_unit_test(TestKoLcmsColorProfile TESTNAME libs-pigment-TestKoLcmsColorProfile  ${TestKoLcmsColorProfile_test_SRCS})

target_link_libraries(TestKoLcmsColorProfile  ${KDE4_KDEUI_LIBS} pigmentcms ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestKoColorSpaceRegistry_test_SRCS TestKoColorSpaceRegistry.cpp )

kde4_add_unit_test(TestKoColorSpaceRegistry TESTNAME libs-pigment-TestKoColorSpaceRegistry  ${TestKoColorSpaceRegistry_test_SRCS})

target_link_libraries(TestKoColorSpaceRegistry  ${KDE4_KDEUI_LIBS} pigmentcms ${LCMS_LIBRARIES} ${QT_QTTEST_LIBRARY})

