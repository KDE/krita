add_subdirectory(tests)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/config)

set(kritawidgetutils_LIB_SRCS
    KoGroupButton.cpp
    KoProgressBar.cpp
    KoProgressUpdater.cpp
    KoUpdater.cpp
    KoUpdaterPrivate_p.cpp
    KoProperties.cpp
    KoFileDialog.cpp

    config/kcolorscheme.cpp         
    config/kcolorschememanager.cpp  
    config/khelpclient.cpp      
    config/klanguagebutton.cpp  
    config/krecentfilesaction.cpp
    config/kstandardaction.cpp

)

add_library(kritawidgetutils SHARED ${kritawidgetutils_LIB_SRCS})
generate_export_header(kritawidgetutils BASE_NAME kritawidgetutils)

target_link_libraries(kritawidgetutils
    PUBLIC
        Qt5::Widgets
        Qt5::Gui
        Qt5::Xml
        Qt5::Core
    PRIVATE
        KF5::I18n
        KF5::ConfigCore
        KF5::ConfigGui
        KF5::GuiAddons
        KF5::WidgetsAddons
)

set_target_properties(kritawidgetutils
    PROPERTIES VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)

install(TARGETS kritawidgetutils ${INSTALL_TARGETS_DEFAULT_ARGS})
