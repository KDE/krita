include_directories({KOVERSION_INCLUDES})
add_subdirectory(tests)

########### libkritastore ###############

if( Qca-qt5_FOUND )
    add_definitions( -DQCA2 )
endif()

set(kritastore_LIB_SRCS
    KoDirectoryStore.cpp
    KoEncryptedStore.cpp
    KoEncryptionChecker.cpp
    KoLZF.cpp
    KoStore.cpp
    KoTarStore.cpp
    KoXmlNS.cpp
    KoXmlReader.cpp
    KoXmlWriter.cpp
    KoZipStore.cpp
    StoreDebug.cpp
    KoNetAccess.cpp # temporary while porting
)

add_library(kritastore SHARED ${kritastore_LIB_SRCS})
generate_export_header(kritastore BASE_NAME kritastore)

target_link_libraries(kritastore Qt5::Xml Qt5::Gui KF5::Archive KF5::Wallet KF5::KIOWidgets KF5::I18n KF5::KIOCore)
#target_link_libraries(kritastore LINK_INTERFACE_LIBRARIES KF5::I18n Qt5::PrintSupport Qt5::Gui Qt5::Xml)
if( Qca-qt5_FOUND )
    target_link_libraries(kritastore qca-qt5)
endif()

set_target_properties(kritastore PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS kritastore ${INSTALL_TARGETS_DEFAULT_ARGS} )

