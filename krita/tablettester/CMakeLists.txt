include_directories(${QT_INCLUDES})
add_executable(KritaTabletTester TabletTester.cpp) 
target_link_libraries(KritaTabletTester ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
install(TARGETS KritaTabletTester ${INSTALL_TARGETS_DEFAULT_ARGS})
