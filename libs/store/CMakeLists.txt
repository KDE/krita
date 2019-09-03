include_directories(${QUAZIP_INCLUDE_DIRS})

add_subdirectory(tests)

set(kritastore_LIB_SRCS
    KoDirectoryStore.cpp
    KoStoreDevice.cpp
    KoLZF.cpp
    KoStore.cpp
    KoXmlNS.cpp
    KoXmlReader.cpp
    KoXmlWriter.cpp
    KoQuaZipStore.cpp
    KoLegacyZipStore.cpp
    StoreDebug.cpp
)

kis_add_core_library(kritastore SHARED ${kritastore_LIB_SRCS})
generate_export_header(kritastore BASE_NAME kritastore)

target_link_libraries(kritastore 
    PRIVATE
        kritaversion 
        kritaglobal 
        KF5::ConfigCore
        KF5::Archive
        Qt5::Xml 
        Qt5::Gui 
        ${QUAZIP_LIBRARIES}
)

set_target_properties(kritastore PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)
install(TARGETS kritastore ${INSTALL_TARGETS_DEFAULT_ARGS} )

