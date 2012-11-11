add_subdirectory(barcode)

if(BUILD_chartshape AND NOT WIN32)
  add_subdirectory(chart)
endif(BUILD_chartshape AND NOT WIN32)

if(MARBLE_FOUND)
  macro_optional_add_subdirectory(maps)
endif(MARBLE_FOUND)

if(QT_QTWEBKIT_FOUND)
  add_subdirectory(web)
endif(QT_QTWEBKIT_FOUND)
