
set(kritarulerassistantcommon_PART_SRCS
    Ruler.cc
    RulerAssistant.cc
    RulerDecoration.cc
     )

kde4_add_library(KritaRulerAssistantCommon SHARED ${kritarulerassistantcommon_PART_SRCS})

target_link_libraries(KritaRulerAssistantCommon kritaui)

install(TARGETS KritaRulerAssistantCommon ${INSTALL_TARGETS_DEFAULT_ARGS})
