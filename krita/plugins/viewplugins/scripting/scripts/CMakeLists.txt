install(FILES scripts.rc DESTINATION ${DATA_INSTALL_DIR}/krita/scripts)

install(FILES
    pilexport.py pilimport.py pilimport.ui
    invert.py invert.rb
    randompaint.rb
    filterstest.rb
    changecs.rb
    reshapehisto.py
    torture-painting.rb torture-filters.rb
    DESTINATION ${DATA_INSTALL_DIR}/krita/scripts/extensions)
