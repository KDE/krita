#make it a project so cmake spits out a codeblocks project
#project(reports)

if(KDE4_BUILD_TESTS)
    # only with this definition will the KOREPORT_TEST_EXPORT macro do something
    add_definitions(-DCOMPILING_TESTS)
endif(KDE4_BUILD_TESTS)

add_subdirectory(pics)
add_subdirectory(tests)

include_directories( ${KOMAIN_INCLUDES}
    ${CMAKE_SOURCE_DIR}/libs/
    ${CMAKE_SOURCE_DIR}/libs/koreport
    ${CMAKE_SOURCE_DIR}/libs/koreport/common
    ${CMAKE_SOURCE_DIR}/libs/koreport/renderer
    ${CMAKE_SOURCE_DIR}/libs/koreport/wrtembed
)

#build a shared library
set(koreport_LIB_SRCS 
    common/krutils.cpp
    common/krpos.cpp
    common/krsize.cpp
    common/KoReportItemBase.cpp
    common/KoReportASyncItemBase.cpp
    common/krsectiondata.cpp
    common/labelsizeinfo.cpp
    common/reportpageoptions.cpp
    common/krreportdata.cpp
    common/krdetailsectiondata.cpp
    common/KoReportPluginInterface.cpp
    common/KoReportItemLine.cpp
    common/renderobjects.cpp
    common/KoReportPluginManager.cpp
    common/KoReportPluginInfo.cpp
    common/KoReportData.cpp

    renderer/KoReportPrintRenderer.cpp
    renderer/KoReportPreRenderer.cpp
    renderer/KoReportASyncItemManager.cpp
    renderer/KoReportScreenRenderer.cpp
    renderer/KoReportHTMLTableRenderer.cpp
    renderer/KoReportHTMLCSSRenderer.cpp
    renderer/KoReportKSpreadRenderer.cpp
    renderer/KoReportODTRenderer.cpp
    renderer/KoOdtFrameReportRenderer.cpp
    renderer/KoReportRendererBase.cpp
    renderer/KoReportPage.cpp

    renderer/scripting/krscripthandler.cpp
    renderer/scripting/krscriptconstants.cpp
    renderer/scripting/krscriptdebug.cpp
    renderer/scripting/krscriptdraw.cpp
    renderer/scripting/krscriptreport.cpp
    renderer/scripting/krscriptsection.cpp
    renderer/scripting/krscriptline.cpp

    renderer/odtframe/KoOdtFrameReportDocument.cpp
    renderer/odtframe/KoOdtFrameReportCheckBox.cpp
    renderer/odtframe/KoOdtFrameReportImage.cpp
    renderer/odtframe/KoOdtFrameReportTextBox.cpp
    renderer/odtframe/KoOdtFrameReportLine.cpp
    renderer/odtframe/KoOdtFrameReportPicture.cpp
    renderer/odtframe/KoOdtFrameReportPrimitive.cpp

    renderer/ods/KoSimpleOdsDocument.cpp
    renderer/ods/KoSimpleOdsSheet.cpp
    renderer/ods/KoSimpleOdsCell.cpp

    wrtembed/detailgroupsectiondialog.cpp
    wrtembed/KoReportDesignerItemBase.cpp
    wrtembed/KoReportDesignerItemRectBase.cpp
    wrtembed/KoReportDesignerItemLine.cpp
    wrtembed/reportsection.cpp
    wrtembed/reportsectiondetailgroup.cpp
    wrtembed/reportsectiondetail.cpp
    wrtembed/reportscene.cpp
    wrtembed/reportsceneview.cpp
    wrtembed/KoReportDesigner.cpp
    wrtembed/reportpropertiesbutton.cpp
    wrtembed/sectioneditor.cpp

    items/label/KoReportItemLabel.cpp
    items/label/KoReportDesignerItemLabel.cpp
    items/label/krscriptlabel.cpp
    items/label/KoReportLabelPlugin.cpp

    items/check/KoReportItemCheck.cpp
    items/check/KoReportDesignerItemCheck.cpp
    items/check/KoReportScriptCheck.cpp
    items/check/KoReportCheckPlugin.cpp

    items/field/KoReportItemField.cpp
    items/field/KoReportDesignerItemField.cpp
    items/field/krscriptfield.cpp
    items/field/KoReportFieldPlugin.cpp

    items/image/KoReportItemImage.cpp
    items/image/KoReportDesignerItemImage.cpp
    items/image/krscriptimage.cpp
    items/image/KoReportImagePlugin.cpp

    items/text/KoReportItemText.cpp
    items/text/KoReportDesignerItemText.cpp
    items/text/krscripttext.cpp
    items/text/KoReportTextPlugin.cpp
    )

kde4_add_ui_files(koreport_LIB_SRCS wrtembed/detailgroupsectiondialog.ui wrtembed/sectioneditor.ui )

kde4_add_library(koreport SHARED ${koreport_LIB_SRCS})

target_link_libraries(koreport
  ${KOPROPERTY_LIBS} ${KDE4_KROSSCORE_LIBS} komain)

target_link_libraries(koreport LINK_INTERFACE_LIBRARIES
  ${KOPROPERTY_LIBS} ${KDE4_KROSSCORE_LIBS} komain)

set_target_properties(koreport PROPERTIES VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION} )
install(TARGETS koreport  ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES  koreport_itemplugin.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})

