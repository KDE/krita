kde4_add_plugin(kexidb_sqlite3_icu icu.c)

include_directories(${ICU_INCLUDE_DIRS})

target_link_libraries(kexidb_sqlite3_icu
                      ${SQLITE_LIBRARIES}
                      ${ICU_LIBRARIES}
                      ${ICU_I18N_LIBRARIES}
)
set_target_properties(kexidb_sqlite3_icu PROPERTIES PREFIX "" DEFINE_SYMBOL libsqlite3_EXPORTS)

install(TARGETS kexidb_sqlite3_icu DESTINATION ${PLUGIN_INSTALL_DIR})
