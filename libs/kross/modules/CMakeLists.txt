include_directories( ${CMAKE_SOURCE_DIR}/libs/kross ${KDE4_INCLUDES} )

# the forms module
set(krossmoduleforms_PART_SRCS form.cpp)
kde4_automoc(krossmoduleforms ${krossmoduleforms_PART_SRCS})
kde4_add_plugin(krossmoduleforms ${krossmoduleforms_PART_SRCS})
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} krossmoduleforms)
target_link_libraries(krossmoduleforms ${QT_QTDESIGNER_LIBRARY} ${KDE4_KDECORE_LIBS} krosscore)
#target_link_libraries(krosscore  ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KNEWSTUFF_LIBS})
install(TARGETS krossmoduleforms DESTINATION ${PLUGIN_INSTALL_DIR})
