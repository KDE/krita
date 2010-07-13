
include_directories( ${KOMAIN_INCLUDES} ${KOTEXT_INCLUDES} )

#find_package(Qt4 4.5.0 REQUIRED)
#INCLUDE(${QT_USE_FILE})

########### next target ###############

set(kofficegoogledocs_PART_SRCS
    onlinedocument.cpp
    googlecontenthandler.cpp
    googledocument.cpp
    googledocumentlist.cpp
    googledocumentservice.cpp
    documentlistwindow.cpp
    loginwindow.cpp
)

kde4_add_ui_files( kofficegoogledocs_PART_SRCS
   authenticationdialog.ui
   documentlist.ui
)
kde4_add_plugin(kofficegoogledocs ${kofficegoogledocs_PART_SRCS})

target_link_libraries(kofficegoogledocs ${QT_LIBRARIES} komain )

install(TARGETS kofficegoogledocs  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  googledocs-kpresenter.rc  DESTINATION ${DATA_INSTALL_DIR}/kpresenter/kpartplugins)
install( FILES  googledocs-kword.rc  DESTINATION ${DATA_INSTALL_DIR}/kword/kpartplugins)


