project(kritaquicklook)

add_custom_target( kritaquicklook ALL xcodebuild -project "${CMAKE_CURRENT_SOURCE_DIR}/../kritaquicklook.xcodeproj" -target kritaquicklook -configuration Release COMMENT build kra qlgenerator )

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/Release/kritaquicklook.qlgenerator DESTINATION ${PLUGIN_INSTALL_DIR})

