add_subdirectory(barcode)

if(NOT WIN32)
  add_subdirectory(chart)
endif(NOT WIN32)

if(MARBLE_FOUND)
  add_subdirectory(maps)
endif(MARBLE_FOUND)

if(QT_QTWEBKIT_FOUND)
  add_subdirectory(web)
endif(QT_QTWEBKIT_FOUND)
