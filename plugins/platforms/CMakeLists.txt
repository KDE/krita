if (HAVE_X11)
    add_subdirectory( xcb )
endif()

if (HAVE_WAYLAND)
    add_subdirectory( wayland )
endif()
