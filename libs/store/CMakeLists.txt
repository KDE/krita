add_subdirectory(tests)

set(kritastore_LIB_SRCS
    KoDirectoryStore.cpp
    KoStoreDevice.cpp
    KoLZF.cpp
    KoStore.cpp
    KoXmlNS.cpp
    KoXmlReader.cpp
    KoXmlWriter.cpp
    KoZipStore.cpp
    StoreDebug.cpp
)

add_library(kritastore SHARED ${kritastore_LIB_SRCS})
generate_export_header(kritastore BASE_NAME kritastore)

target_link_libraries(kritastore kritaversion kritaglobal Qt5::Xml Qt5::Gui KF5::Archive)

set_target_properties(kritastore PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)
install(TARGETS kritastore ${INSTALL_TARGETS_DEFAULT_ARGS} )

