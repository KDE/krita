add_subdirectory(tests)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/libkdcraw/src)

set(krita_raw_import_SOURCES
    kis_raw_import.cpp 
    3rdparty/libkdcraw/src/dcrawsettingswidget.cpp
    3rdparty/libkdcraw/src/libkdcraw_debug.cpp
    3rdparty/libkdcraw/src/rcombobox.cpp
    3rdparty/libkdcraw/src/rexpanderbox.cpp
    3rdparty/libkdcraw/src/rnuminput.cpp
    3rdparty/libkdcraw/src/rsliderspinbox.cpp
    3rdparty/libkdcraw/src/rwidgetutils.cpp
)

ki18n_wrap_ui(krita_raw_import_SOURCES 
    wdgrawimport.ui 
)

kis_add_library(krita_raw_import MODULE ${krita_raw_import_SOURCES})
target_link_libraries(krita_raw_import kritaui KF6::KDcraw kritaexifcommon)

install(TARGETS krita_raw_import  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( PROGRAMS  krita_raw.desktop  DESTINATION ${KDE_INSTALL_APPDIR})

