include(KritaAddBrokenUnitTest)

kis_add_test(
    ../KoLZF.cpp TestKoLZF.cpp
    TEST_NAME TestKoLZF
    LINK_LIBRARIES kritastore kritatestsdk kritaglobal
    NAME_PREFIX "libs-odf"
    )


########### manual test for file contents ###############
add_executable(storedroptest storedroptest.cpp)
target_link_libraries(storedroptest kritastore Qt6::Widgets)
ecm_mark_as_test(storedroptest)

macos_test_fixrpath( storedroptest )
