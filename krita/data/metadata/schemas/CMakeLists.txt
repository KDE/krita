########### install schemas #############
install( FILES
    dc.schema
    exif.schema
    tiff.schema
    mkn.schema
    xmp.schema
    xmpmm.schema
    xmprights.schema

    DESTINATION ${DATA_INSTALL_DIR}/krita/metadata/schemas)
                                                             
