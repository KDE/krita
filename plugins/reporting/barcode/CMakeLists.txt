include_directories(
    ${KOPROPERTY_INCLUDES}
    ${KOREPORT_INCLUDES}
)

#build a shared library
set(koreport_barcodeplugin_LIB_SRCS
    KoReportItemBarcode.cpp
    KoReportDesignerItemBarcode.cpp
    krscriptbarcode.cpp
    3of9.cpp
    i2of5.cpp
    code128.cpp
    codeean.cpp
    ext3of9.cpp
    i2of5paint.cpp
    3of9paint.cpp
    ext3of9paint.cpp
    codeeanpaint.cpp
    code128paint.cpp
    KoReportBarcodePlugin.cpp
)

add_library(koreport_barcodeplugin MODULE ${koreport_barcodeplugin_LIB_SRCS})

target_link_libraries(koreport_barcodeplugin   ${KOREPORT_LIBS} ${KOPROPERTY_LIBS})


########### install files ###############

install(TARGETS koreport_barcodeplugin  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES  koreport_barcodeplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
ecm_install_icons(${DATA_INSTALL_DIR}/calligra/icons)
