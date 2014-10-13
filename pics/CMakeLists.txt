add_subdirectory( hicolor )
add_subdirectory( oxygen )

install( FILES
    dark_docker_lock_a.png
    dark_docker_lock_b.png

    light_docker_lock_a.png
    light_docker_lock_b.png
    DESTINATION ${DATA_INSTALL_DIR}/calligra/pics)
