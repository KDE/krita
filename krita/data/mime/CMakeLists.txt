install( FILES
    krita.xml
    x-gimp-brush-animated.xml
    x-gimp-brush.xml
    DESTINATION ${XDG_MIME_INSTALL_DIR}
)
if (SHARED_MIME_INFO_FOUND)
    update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})
endif()
