add_subdirectory(barcode)

if(NOT WIN32)
  add_subdirectory(chart)
endif()

if(MARBLE_FOUND)
  add_subdirectory(maps)
endif()

if(QT_QTWEBKIT_FOUND)
  add_subdirectory(web)
endif()
