include_directories( ${KOMAIN_INCLUDES} ${KOTEXT_INCLUDES} )

#find_package(Qt4 4.5.0 REQUIRED)
#INCLUDE(${QT_USE_FILE})

########### next target ###############

SET(onlinedocs_RCCS onlineservices.qrc)
QT4_ADD_RESOURCES(onlinedocs_RCC_SRCS ${onlinedocs_RCCS})

set(calligragoogledocs_PART_SRCS
    onlinedocument.cpp
    googlecontenthandler.cpp
    googledocument.cpp
    googledocumentlist.cpp
    googledocumentservice.cpp
    documentlistwindow.cpp
    loginwindow.cpp
    ${onlinedocs_RCC_SRCS}
)

kde4_add_ui_files( calligragoogledocs_PART_SRCS
   authenticationdialog.ui
   documentlist.ui
)
kde4_add_plugin(calligragoogledocs ${calligragoogledocs_PART_SRCS})

target_link_libraries(calligragoogledocs ${QT_LIBRARIES} komain )

install(TARGETS calligragoogledocs  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  googledocs-stage.rc  DESTINATION ${DATA_INSTALL_DIR}/stage/kpartplugins)
install( FILES  googledocs-sheets.rc  DESTINATION ${DATA_INSTALL_DIR}/sheets/kpartplugins)
install( FILES  googledocs-words.rc  DESTINATION ${DATA_INSTALL_DIR}/words/kpartplugins)


