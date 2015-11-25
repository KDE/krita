project(quicklook)

set(kritaquicklook_SRCS
    GeneratePreviewForURL.c
    GenerateThumbnailForURL.c
    main.c
)

set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "-bundle -headerpad_max_install_names")
add_library(kritaquicklook MODULE ${kritaquicklook_SRCS})

find_library(FOUNDATION_LIBRARY Foundation)

target_link_libraries(kritaquicklook
    KF5::Archive
    Qt5::Core
    ${FOUNDATION_LIBRARY}
)

set_target_properties(kritaquicklook PROPERTIES
    FRAMEWORK TRUE
    MACOSX_FRAMEWORK_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
    SUFFIX .qlgenerator
)

install(TARGETS kritaquicklook  DESTINATION ${PLUGIN_INSTALL_DIR})

