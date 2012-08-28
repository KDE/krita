

include_directories( ${CMAKE_SOURCE_DIR}/kexi ${CMAKE_SOURCE_DIR}/libs ${KDE4_INCLUDES} )


########### next target ###############

set(keximigrate_spreadsheet_PART_SRCS spreadsheetmigrate.cpp )


kde4_add_plugin(keximigrate_spreadsheet ${keximigrate_spreadsheet_PART_SRCS})



target_link_libraries(keximigrate_spreadsheet
    ${KDE4_KPARTS_LIBS}
    keximigrate
    kexidb
    kexiutils
    ${CALLIGRADB_LIBS}
    calligrasheetscommon
)

install(TARGETS keximigrate_spreadsheet DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  keximigrate_spreadsheet.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
