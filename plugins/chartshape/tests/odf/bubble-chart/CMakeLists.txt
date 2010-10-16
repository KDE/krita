set( TestLoading_SRCS TestLoading.cpp ../TestLoadingBase.cpp ../../../ChartDocument.cpp )
kde4_add_unit_test( TestLoading TESTNAME kchart-TestLoading-bubble-chart ${TestLoading_SRCS} )
target_link_libraries( TestLoading ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} flake koodf chartshapelib komain )
