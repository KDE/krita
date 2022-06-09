set(KRITA_OVERVIEWDOCKER_SOURCES overviewdocker.cpp
    overviewdocker_dock.cpp
    overviewwidget.cc
    OverviewThumbnailStrokeStrategy.cpp)
kis_add_library(kritaoverviewdocker MODULE ${KRITA_OVERVIEWDOCKER_SOURCES} PCH_FILE kis_image_pch.h)
target_link_libraries(kritaoverviewdocker kritaui)
install(TARGETS kritaoverviewdocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
