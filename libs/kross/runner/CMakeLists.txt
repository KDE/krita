
set(krossrunner_SRCS main.cpp )

kde4_automoc(${krossrunner_SRCS})

kde4_add_executable(krossrunner ${krossrunner_SRCS})

target_link_libraries(krossrunner  ${KDE4_KDECORE_LIBS} krossapi krossmain )

install(TARGETS krossrunner  DESTINATION ${BIN_INSTALL_DIR})
