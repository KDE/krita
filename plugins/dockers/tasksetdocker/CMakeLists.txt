set(KRITA_TASKSETDOCKER_SOURCES tasksetmodel.cpp tasksetdocker.cpp tasksetdocker_dock.cpp taskset_resource.cpp)

ki18n_wrap_ui(KRITA_TASKSETDOCKER_SOURCES
    wdgtasksetdocker.ui
)
kis_add_library(kritatasksetdocker MODULE ${KRITA_TASKSETDOCKER_SOURCES})
target_link_libraries(kritatasksetdocker kritaui)
install(TARGETS kritatasksetdocker DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
