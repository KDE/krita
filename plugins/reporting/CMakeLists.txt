add_subdirectory(barcode)

if(BUILD_chartshape AND NOT WIN32)
  add_subdirectory(chart)
endif(BUILD_chartshape AND NOT WIN32)

if(MARBLE_FOUND AND Soprano_FOUND)
  add_subdirectory(maps)
endif(MARBLE_FOUND AND Soprano_FOUND)

if(QT_QTWEBKIT_FOUND)
  add_subdirectory(web)
endif(QT_QTWEBKIT_FOUND)
