set(kritasnapshotdocker_SOURCES
  KisSnapshotModel.cpp
  SnapshotDocker.cpp
  SnapshotPlugin.cpp
  KisSnapshotView.cpp
  )

kis_add_library(kritasnapshotdocker MODULE ${kritasnapshotdocker_SOURCES})
target_link_libraries(kritasnapshotdocker kritaimage kritaui)
install(TARGETS kritasnapshotdocker DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})