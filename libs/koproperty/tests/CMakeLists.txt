if(KDE4_BUILD_TESTS)

  include_directories( ${CMAKE_SOURCE_DIR}/libs/koproperty ${KOPROPERTY_INCLUDES} )

  set(propertytest_SRCS main.cpp test.cpp )
  
  add_executable(propertytest NOGUI ${propertytest_SRCS})
  
  target_link_libraries(propertytest   koproperty )

endif()
