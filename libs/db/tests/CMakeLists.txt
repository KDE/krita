calligra_build_test(Utils)
target_link_libraries(TestUtils ${CALLIGRADB_LIBS})

calligra_build_test(Connection)
target_link_libraries(TestConnection ${CALLIGRADB_LIBS})
