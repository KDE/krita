add_library(kexidb_sqlite3_icu SHARED icu.c)

target_link_libraries(kexidb_sqlite3_icu
                      ${SQLITE_LIBRARIES}
                      ${ICU_LIBRARIES}
                      ${ICU_I18N_LIBRARIES}
)
set_target_properties(kexidb_sqlite3_icu PROPERTIES PREFIX "" DEFINE_SYMBOL libsqlite3_EXPORTS)

install(TARGETS kexidb_sqlite3_icu DESTINATION ${PLUGIN_INSTALL_DIR})
