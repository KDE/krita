add_subdirectory(tests)

configure_file(xmlgui/config-xmlgui.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-xmlgui.h )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/config)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/xmlgui)

set(kritawidgetutils_LIB_SRCS

    WidgetUtilsDebug.cpp

    kis_icon_utils.cpp
    kis_action_registry.cpp
    KisActionsSnapshot.cpp
    KoGroupButton.cpp
    KoProgressProxy.cpp
    KoFakeProgressProxy.cpp
    KoProgressBar.cpp
    KoProgressUpdater.cpp
    KoUpdater.cpp
    KoUpdaterPrivate_p.cpp
    KoProperties.cpp
    KoFileDialog.cpp
    KoResourcePaths.cpp
    KisKineticScroller.cpp
    KisSqueezedComboBox.cpp

    kis_num_parser.cpp
    kis_spin_box_unit_manager.cpp

    config/kcolorscheme.cpp
    config/kcolorschememanager.cpp
    config/khelpclient.cpp
    config/klanguagebutton.cpp
    config/krecentfilesaction.cpp
    config/kstandardaction.cpp

    xmlgui/KisShortcutsEditorItem.cpp
    xmlgui/KisShortcutEditWidget.cpp
    xmlgui/KisShortcutsEditorDelegate.cpp
    xmlgui/KisShortcutsDialog.cpp
    xmlgui/KisShortcutsDialog_p.cpp
    xmlgui/KisShortcutsEditor.cpp
    xmlgui/KisShortcutsEditor_p.cpp

    xmlgui/kshortcutschemeseditor.cpp
    xmlgui/kshortcutschemeshelper.cpp

    xmlgui/kaboutkdedialog_p.cpp
    xmlgui/kactioncategory.cpp
    xmlgui/kactioncollection.cpp
    xmlgui/kbugreport.cpp
    xmlgui/kcheckaccelerators.cpp
    xmlgui/kedittoolbar.cpp
    xmlgui/kgesture.cpp
    xmlgui/kgesturemap.cpp
    xmlgui/khelpmenu.cpp
    xmlgui/kkeysequencewidget.cpp
    xmlgui/kmainwindow.cpp
    xmlgui/kmenumenuhandler_p.cpp
    xmlgui/kshortcutwidget.cpp
    xmlgui/kswitchlanguagedialog_p.cpp
    xmlgui/ktoggletoolbaraction.cpp
    xmlgui/ktoolbar.cpp
    xmlgui/ktoolbarhandler.cpp
    xmlgui/kundoactions.cpp
    xmlgui/kxmlguibuilder.cpp
    xmlgui/kxmlguiclient.cpp
    xmlgui/kxmlguifactory.cpp
    xmlgui/kxmlguifactory_p.cpp
    xmlgui/kxmlguiversionhandler.cpp
    xmlgui/kxmlguiwindow.cpp
)

if (HAVE_DBUS)
    set(kritawidgetutils_LIB_SRCS ${kritawidgetutils_LIB_SRCS}
        xmlgui/kmainwindowiface.cpp
    )
endif()

ki18n_wrap_ui(kritawidgetutils_LIB_SRCS
    xmlgui/KisShortcutsDialog.ui
    xmlgui/kshortcutwidget.ui
)

qt5_add_resources(kritawidgetutils_LIB_SRCS xmlgui/kxmlgui.qrc)

add_library(kritawidgetutils SHARED ${kritawidgetutils_LIB_SRCS})

target_include_directories(kritawidgetutils
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/config>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/xmlgui>
)

generate_export_header(kritawidgetutils BASE_NAME kritawidgetutils)

if (HAVE_DBUS)
   set (KRITA_WIDGET_UTILS_EXTRA_LIBS ${KRITA_WIDGET_UTILS_EXTRA_LIBS} Qt5::DBus)
endif ()

if (APPLE)
    find_library(FOUNDATION_LIBRARY Foundation)
    set(KRITA_WIDGET_UTILS_EXTRA_LIBS ${KRITA_WIDGET_UTILS_EXTRA_LIBS} ${FOUNDATION_LIBRARY})
endif ()

target_link_libraries(kritawidgetutils
    PUBLIC
        Qt5::Widgets
        Qt5::Gui
        Qt5::Xml
        Qt5::Core
        KF5::ItemViews
        kritaglobal
    PRIVATE
        Qt5::PrintSupport
        KF5::I18n
        KF5::ConfigCore
        KF5::CoreAddons
        KF5::ConfigGui
        KF5::GuiAddons
        KF5::WidgetsAddons
        KF5::WindowSystem
        kritaplugin
        kritaodf
        ${KRITA_WIDGET_UTILS_EXTRA_LIBS}
)

set_target_properties(kritawidgetutils
    PROPERTIES VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

install(TARGETS kritawidgetutils ${INSTALL_TARGETS_DEFAULT_ARGS})
