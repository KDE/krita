

include_directories( ${SQLITE_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/kexi ${CMAKE_SOURCE_DIR}/kexi/kexidb ${KDE4_INCLUDES}  )

ADD_DEFINITIONS( -D__KEXIDB__= )

########### next target ###############

set(kexidb_sqlite3driver_PART_SRCS 
   sqliteconnection.cpp 
   sqlitedriver.cpp 
   sqlitecursor.cpp 
   sqlitekeywords.cpp 
   sqlitepreparedstatement.cpp
   sqliteadmin.cpp
   sqlitevacuum.cpp
   sqlitealter.cpp
)

kde4_add_plugin(kexidb_sqlite3driver ${kexidb_sqlite3driver_PART_SRCS})

target_link_libraries(kexidb_sqlite3driver ${KDE4_KPARTS_LIBS} ${SQLITE_LIBRARIES} ${CALLIGRADB_LIBS})

install(TARGETS kexidb_sqlite3driver  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kexidb_sqlite3driver.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(icu)
