include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )

add_subdirectory(tests)

set(pigment_SRCS
    KoColor.cpp
)

kde4_automoc(${pigment_SRCS})

kde4_add_library(pigment SHARED ${pigment_SRCS})

target_link_libraries(pigment ${KDE4_KDEUI_LIBS})

set_target_properties(pigment PROPERTIES VERSION 1.0.0 SOVERSION 1 )


install_targets(${LIB_INSTALL_DIR} pigment )


