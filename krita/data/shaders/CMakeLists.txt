install( FILES  
    bilinear_gradient.frag
    conical_gradient.frag
    conical_symetric_gradient.frag

    matrix_transform.vert
    matrix_transform_legacy.vert

    highq_downscale_preface.frag.inc
    highq_downscale_main.frag.inc
    highq_downscale_main_ocio.frag.inc

    simple_texture_preface.frag.inc
    simple_texture_main.frag.inc
    simple_texture_main_ocio.frag.inc

    simple_texture_legacy_preface.frag.inc
    simple_texture_legacy_main.frag.inc
    simple_texture_legacy_main_ocio.frag.inc

    cursor.vert
    cursor.frag
    linear_gradient.frag
    radial_gradient.frag
    square_gradient.frag
    DESTINATION ${DATA_INSTALL_DIR}/krita/shaders)

