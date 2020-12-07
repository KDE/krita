if (APPLE)
project(kritaintegration)

add_custom_target( kritaintegration ALL xcodebuild -project "${CMAKE_CURRENT_SOURCE_DIR}/integration.xcodeproj" -alltargets -configuration Release COMMENT build all macOS plugins )

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Build/Release/kritaquicklook.qlgenerator DESTINATION ${PLUGIN_INSTALL_DIR})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Build/Release/kritaspotlight.mdimporter DESTINATION ${PLUGIN_INSTALL_DIR})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Build/Release/kritaquicklookng.appex DESTINATION ${PLUGIN_INSTALL_DIR})
endif(APPLE)
