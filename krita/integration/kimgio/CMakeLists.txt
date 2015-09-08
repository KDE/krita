add_library(kimg_kra MODULE kra.cpp)
target_link_libraries(kimg_kra Qt5::Gui KF5::Archive)

add_library(kimg_ora MODULE ora.cpp)
target_link_libraries(kimg_ora Qt5::Gui KF5::Archive)

install(TARGETS kimg_ora  DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/imageformats/)
install(FILES ora.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/qimageioplugins/)

install(TARGETS kimg_kra  DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/imageformats/)
install(FILES kra.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/qimageioplugins/)
