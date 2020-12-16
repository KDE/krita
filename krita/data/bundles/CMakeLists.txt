set (krita_BUNDLE_FILES
     README
     Krita_4_Default_Resources.bundle
     Krita_Artists_SeExpr_examples.bundle
     RGBA_brushes.bundle
)

if (NOT ANDROID)
    set (krita_BUNDLE_FILES
         ${krita_BUNDLE_FILES}
         Krita_3_Default_Resources.bundle
         )
endif()

install( FILES
    ${krita_BUNDLE_FILES}
    DESTINATION  ${DATA_INSTALL_DIR}/krita/bundles)
