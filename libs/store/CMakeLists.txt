include_directories({KOVERSION_INCLUDES})
add_subdirectory(tests)

########### libkostore ###############

if( Qca-qt5_FOUND )
    add_definitions( -DQCA2 )
endif()

set(kostore_LIB_SRCS
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
)

add_library(kostore SHARED ${kostore_LIB_SRCS})
generate_export_header(kostore BASE_NAME kostore)

target_link_libraries(kostore KF5::Archive KF5::Wallet KF5::KIOWidgets KF5::I18n KF5::KDELibs4Support)
target_link_libraries(kostore LINK_INTERFACE_LIBRARIES KF5::I18n Qt5::PrintSupport Qt5::Gui Qt5::Xml)
if( Qca-qt5_FOUND )
    target_link_libraries(kostore qca-qt5)
endif()

set_target_properties(kostore PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS kostore ${INSTALL_TARGETS_DEFAULT_ARGS} )


install( FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kostore_export.h
    KoStore.h
DESTINATION ${INCLUDE_INSTALL_DIR}/calligra COMPONENT Devel)
