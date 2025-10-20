if (APPLE)
project(kritaintegration)

if (NOT MACOS_UNIVERSAL)
    set(XCODEBUILD_ARCHS "-arch" "${CMAKE_SYSTEM_PROCESSOR}")
endif()

add_custom_target( kritaintegration ALL xcodebuild
    -project "${CMAKE_CURRENT_SOURCE_DIR}/integration.xcodeproj" ${XCODEBUILD_ARCHS}
    -target kritaquicklook
    -target kritaspotlight
    -target krita-thumbnailer
    -target krita-preview
    -configuration Release COMMENT build all macOS plugins
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Build/Release/kritaquicklook.qlgenerator DESTINATION ${KDE_INSTALL_PLUGINDIR})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Build/Release/kritaspotlight.mdimporter DESTINATION ${KDE_INSTALL_PLUGINDIR})

# TODO fix and reenable - https://bugs.kde.org/show_bug.cgi?id=430553
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Build/Release/krita-thumbnailer.appex DESTINATION ${KDE_INSTALL_PLUGINDIR})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Build/Release/krita-preview.appex DESTINATION ${KDE_INSTALL_PLUGINDIR})
endif(APPLE)
