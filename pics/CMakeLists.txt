add_subdirectory( hicolor )
add_subdirectory( oxygen )

install( FILES
    dark_docker_lock_a.png
    dark_docker_lock_b.png
    dark_docker_close.png
    dark_docker_collapse_a.png
    dark_docker_collapse_b.png
    dark_docker_float.png

    light_docker_lock_a.png
    light_docker_lock_b.png
    light_docker_close.png
    light_docker_collapse_a.png
    light_docker_collapse_b.png
    light_docker_float.png

    DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
