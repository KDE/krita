include_directories(
    ${KOMAIN_INCLUDES}
)
set(filedialogtester_SRCS
    KoFileDialogTester.cpp
    main.cpp
)
kde4_add_ui_files(filedialogtester_SRCS
    KoFileDialogTester.ui
)

kde4_add_executable(filedialogtester ${filedialogtester_SRCS})
target_link_libraries(filedialogtester kowidgets)
install(TARGETS filedialogtester  ${INSTALL_TARGETS_DEFAULT_ARGS})

