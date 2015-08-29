set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

########### next target ###############
set(scratchpad_SRCS scratchpad.cpp)
add_executable(scratchpad ${scratchpad_SRCS})
target_link_libraries(scratchpad   kritaui )
