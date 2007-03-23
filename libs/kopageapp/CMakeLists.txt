include_directories(${KOFFICEUI_INCLUDES} ${KOTEXT_INCLUDES})

if(KDE4_BUILD_TESTS)
    # only with this definition will the KOPAGEAPP_TEST_EXPORT macro do something
    add_definitions(-DCOMPILING_TESTS)
endif(KDE4_BUILD_TESTS)


add_subdirectory(tests)

### kopageapp ###
set( kopageapp_LIB_SRCS
     KoPADocument.cpp
     KoPAView.cpp
     KoPACanvas.cpp
     KoPASavingContext.cpp
     KoPAPageBase.cpp
     KoPAMasterPage.cpp
     KoPAPage.cpp
     commands/KoPAPageInsertCommand.cpp
     commands/KoPAPageDeleteCommand.cpp
)

kde4_automoc(${kopageapp_LIB_SRCS})

kde4_add_library(kopageapp SHARED ${kopageapp_LIB_SRCS})

target_link_libraries(kopageapp ${KDE4_KDECORE_LIBS} kofficeui flake kotext)

set_target_properties(kopageapp PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kopageapp  DESTINATION ${LIB_INSTALL_DIR})

