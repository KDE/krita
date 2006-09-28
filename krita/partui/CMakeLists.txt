add_subdirectory(tests)

########### next target ###############

set(kritapartui_LIB_SRCS
    kis_canvas.cpp
    kis_opengl_canvas.cpp
    kis_qpainter_canvas.cpp
    kis_view_converter.cpp
    kis_view.cpp
)

kde4_automoc(${kritapartui_LIB_SRCS})

kde4_add_library(kritapartui SHARED ${kritapartui_LIB_SRCS})

target_link_libraries(kritapartui ${KDE4_KDECORE_LIBS} kritaimage kofficecore kopainter flake kofficeui)

if(HAVE_OPENGL)
    target_link_libraries(kritapartui
        ${OPENGL_LIBRARIES}
        ${QT_QTOPENGL_LIBRARY}
        )
endif(HAVE_OPENGL)

set_target_properties(kritapartui PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kritapartui  DESTINATION ${LIB_INSTALL_DIR})


