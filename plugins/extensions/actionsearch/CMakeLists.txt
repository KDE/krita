set(kritaactionsearch_SOURCES 
    ActionSearch.cpp
    KisActionSearchWidget.cpp
)

ki18n_wrap_ui(kritaactionsearch_SOURCES WdgActionSearch.ui )

add_library(kritaactionsearch MODULE ${kritaactionsearch_SOURCES})

target_link_libraries(kritaactionsearch kritaui)

install(TARGETS kritaactionsearch  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install( FILES  ActionSearch.xmlgui  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)

install( FILES actionsearch.action DESTINATION  ${DATA_INSTALL_DIR}/krita/actions)
