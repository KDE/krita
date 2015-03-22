add_subdirectory(backgroundTool)

install(
    FILES
        kopa_tool.desktop
    DESTINATION
        ${SERVICETYPES_INSTALL_DIR}
)
