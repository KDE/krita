
set(KRITA_TASKSETDOCKER_PART_SRCS tasksetmodel.cpp tasksetdocker.cpp tasksetdocker_dock.cpp taskset_resource.cpp )

ki18n_wrap_ui(KRITA_TASKSETDOCKER_PART_SRCS
    wdgtasksetdocker.ui
)
add_library(kritatasksetdocker MODULE ${KRITA_TASKSETDOCKER_PART_SRCS})
kcoreaddons_desktop_to_json(kritatasksetdocker krita_tasksetdocker.desktop)

target_link_libraries(kritatasksetdocker kritaui)

install(TARGETS kritatasksetdocker  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_tasksetdocker.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

