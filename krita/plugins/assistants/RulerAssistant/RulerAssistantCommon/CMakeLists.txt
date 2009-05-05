
set(kritarulerassistantcommon_PART_SRCS
    Ruler.cc
    RulerAssistant.cc
     )

kde4_add_library(KritaRulerAssistantCommon SHARED ${kritarulerassistantcommon_PART_SRCS})

target_link_libraries(KritaRulerAssistantCommon kritaui ${KDE4_KDEUI_LIBS})

set_target_properties(KritaRulerAssistantCommon PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)

install(TARGETS KritaRulerAssistantCommon ${INSTALL_TARGETS_DEFAULT_ARGS})
