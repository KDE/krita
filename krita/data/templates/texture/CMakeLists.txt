kde4_install_icons(${DATA_INSTALL_DIR}/krita/icons )
install( FILES  
    Texture1k16bitfloatrgb.kra
    Texture1k16bitscalar.kra
    Texture1k32bitscalar.kra
    Texture1k8bitsrgb.kra
    Texture2k16bitfloatrgb.kra
    Texture2k16bitscalar.kra
    Texture2k32bitscalar.kra
    Texture2k8bitsrgb.kra
    Texture4k16bitfloatrgb.kra
    Texture4k16bitscalar.kra
    Texture4k32bitscalar.kra
    Texture4k8bitsrgb.kra
    Texture8k8bitsrgb.kra
DESTINATION ${DATA_INSTALL_DIR}/krita/templates/texture/.source)

install( FILES  
    .directory
    Texture1k16bitfloatrgb.desktop
    Texture1k16bitscalar.desktop
    Texture1k32bitscalar.desktop
    Texture1k8bitsrgb.desktop
    Texture2k16bitfloatrgb.desktop
    Texture2k16bitscalar.desktop
    Texture2k32bitscalar.desktop
    Texture2k8bitsrgb.desktop
    Texture4k16bitfloatrgb.desktop
    Texture4k16bitscalar.desktop
    Texture4k32bitscalar.desktop
    Texture4k8bitsrgb.desktop
    Texture8k16bitfloatrgb.desktop
    Texture8k16bitscalar.desktop
    Texture8k32bitscalar.desktop
    Texture8k8bitsrgb.desktop
DESTINATION ${DATA_INSTALL_DIR}/krita/templates/texture)
