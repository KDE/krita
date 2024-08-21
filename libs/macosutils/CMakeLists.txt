
find_library(FOUNDATION_LIBRARY Foundation)
find_library(SECURITY_LIBRARY Security)
find_library(APPKIT_LIBRARY AppKit)

set(macosutils_LIB_SRCS
    KisMacosSecurityBookmarkManager.mm
    KisMacosEntitlements.mm
    KisMacosSystemProber.mm
)

kis_add_library(kritamacosutils SHARED ${macosutils_LIB_SRCS})

target_link_libraries(kritamacosutils
    PRIVATE
        ${FOUNDATION_LIBRARY}
        ${SECURITY_LIBRARY}
        ${APPKIT_LIBRARY}
        Qt${QT_MAJOR_VERSION}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_MAJOR_VERSION}::Widgets
        KF${KF_MAJOR}::I18n
    )

set_target_properties(kritamacosutils PROPERTIES
    COMPILE_FLAGS "-fobjc-arc")

set_target_properties(kritamacosutils PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)
install(TARGETS kritamacosutils ${INSTALL_TARGETS_DEFAULT_ARGS})
