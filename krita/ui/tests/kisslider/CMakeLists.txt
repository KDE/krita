set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

########### next target ###############
set(slider_SRCS slider.cpp)
kde4_add_executable(slider ${slider_SRCS})
target_link_libraries(slider  ${KDE4_KDEUI_LIBS} kritaui )
