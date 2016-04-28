if (WIN32)
    option(ENABLE_GEMINI_STARTUP_MAGIC "Enable Gemini's windows startup magic" ON)

    if (ENABLE_GEMINI_STARTUP_MAGIC)
        add_definitions(-DGEMINI_ENABLE_STARTUP_MAGIC)
    endif ()
endif ()

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/..
                    ${CMAKE_CURRENT_SOURCE_DIR}/models
                    ${CMAKE_CURRENT_BINARY_DIR})

set(kritagemini_SRCS
    main.cpp
    MainWindow.cpp
    desktopviewproxy.cpp
)


if (WIN32)
    set(kritagemini_SRCS ${kritagemini_SRCS} KritaGeminiWin.rc)
else ()
    ecm_add_app_icon(kritagemini_SRCS ICONS kritagemini.png)
endif ()

# Krita Gemini application
# =============================================================================

add_executable(kritagemini ${kritagemini_SRCS})

target_link_libraries(kritagemini
    Qt5::Declarative
    Qt5::Gui
    Qt5::Core
    ${QT_LIBRARIES}
    kritaui
    kritasketchlib
)

#install(TARGETS   kritagemini ${#install_TARGETS_DEFAULT_ARGS})
#install(PROGRAMS  kritagemini.desktop               DESTINATION ${XDG_APPS_#install_DIR})
#install(FILES     kritageminirc kritageminipanelsrc DESTINATION ${CONFIG_#install_DIR} )
#install(FILES     kritagemini.qml                   DESTINATION ${DATA_#install_DIR}/kritagemini)
#install(FILES     kritagemini.xmlgui                    DESTINATION ${DATA_#install_DIR}/kritagemini)

add_subdirectory(icon)
