add_subdirectory(tests)

if(OPENEXR_FOUND)
	include_directories(${OPENEXR_INCLUDE_DIR})
endif()
include_directories(${LibRaw_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/libkdcraw/src)

set(krita_raw_import_SOURCES kis_raw_import.cpp 
    3rdparty/libkdcraw/src/dcrawinfocontainer.cpp
    3rdparty/libkdcraw/src/dcrawsettingswidget.cpp
    3rdparty/libkdcraw/src/kdcraw.cpp
    3rdparty/libkdcraw/src/kdcraw_p.cpp
    3rdparty/libkdcraw/src/libkdcraw_debug.cpp
    3rdparty/libkdcraw/src/ractionjob.cpp
    3rdparty/libkdcraw/src/ractionthreadbase.cpp
    3rdparty/libkdcraw/src/rawdecodingsettings.cpp
    3rdparty/libkdcraw/src/rcombobox.cpp
    3rdparty/libkdcraw/src/rexpanderbox.cpp
    3rdparty/libkdcraw/src/rnuminput.cpp
    3rdparty/libkdcraw/src/rsliderspinbox.cpp
    3rdparty/libkdcraw/src/rwidgetutils.cpp
    3rdparty/libkdcraw/src/squeezedcombobox.cpp
)

ki18n_wrap_ui(krita_raw_import_SOURCES 
    wdgrawimport.ui 
)

kis_add_library(krita_raw_import MODULE ${krita_raw_import_SOURCES})
target_link_libraries(krita_raw_import kritaui ${LibRaw_LIBRARIES})

install(TARGETS krita_raw_import  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( PROGRAMS  krita_raw.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})

