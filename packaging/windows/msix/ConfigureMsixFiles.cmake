# The MSIX packaging is only designed for x64
if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")

if(DEFINED KRITA_STABLE)
    set(MSIX_DISPLAY_NAME_SUFFIX "")
else()
    set(MSIX_DISPLAY_NAME_SUFFIX " ${KRITA_VERSION_STRING}")
    if(KRITA_GIT_SHA1_STRING)
        set(MSIX_DISPLAY_NAME_SUFFIX "${MSIX_DISPLAY_NAME_SUFFIX} (git ${KRITA_GIT_SHA1_STRING})")
    endif(KRITA_GIT_SHA1_STRING)
endif()

configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/manifest.xml.in
    ${CMAKE_CURRENT_BINARY_DIR}/manifest.xml
    @ONLY
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/manifest.xml
        ${CMAKE_CURRENT_LIST_DIR}/build_msix.cmd
        ${CMAKE_CURRENT_LIST_DIR}/priconfig.xml
    DESTINATION
        ${CMAKE_INSTALL_PREFIX}/krita-msix
)

install(
    DIRECTORY 
        ${CMAKE_CURRENT_LIST_DIR}/pkg
    DESTINATION
        ${CMAKE_INSTALL_PREFIX}/krita-msix
)

endif()
