set(kritasearch_SOURCES 
    SearchWidget.cpp
    SearchPopup.cpp
    SearchModel.cpp
    SearchFilterModel.cpp
    SearchPlugin.cpp
)

ki18n_wrap_ui(kritasearch_SOURCES WdgSearch.ui )

add_library(kritasearch MODULE ${kritasearch_SOURCES})

target_link_libraries(kritasearch kritaui)

install(TARGETS kritasearch  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install( FILES Search.action DESTINATION  ${DATA_INSTALL_DIR}/krita/actions)
