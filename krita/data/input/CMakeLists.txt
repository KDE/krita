install(FILES
    kritadefault.profile
    painttoolsaicompatible.profile
    photoshopcompatible.profile
    DESTINATION  ${DATA_INSTALL_DIR}/krita/input
)
if (WIN32)
   install(FILES
        tabletpro.profile
        DESTINATION  ${DATA_INSTALL_DIR}/krita/input
    )
endif()

