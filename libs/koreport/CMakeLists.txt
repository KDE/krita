#make it a project so cmake spits out a codeblocks project
#project(reports)

add_subdirectory(pics)
add_subdirectory(plugins)

include_directories( ${KOMAIN_INCLUDES}
    ${KOODF_INCLUDES}
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

    renderer/KoReportPrintRenderer.cpp
    renderer/KoReportPreRenderer.cpp
    renderer/KoReportScreenRenderer.cpp
    renderer/KoReportHTMLTableRenderer.cpp
    renderer/KoReportHTMLCSSRenderer.cpp
    renderer/KoReportKSpreadRenderer.cpp
    renderer/KoReportRendererBase.cpp
    renderer/KoReportPage.cpp

    renderer/scripting/krscripthandler.cpp
    renderer/scripting/krscriptconstants.cpp
    renderer/scripting/krscriptdebug.cpp
    renderer/scripting/krscriptdraw.cpp
    renderer/scripting/krscriptreport.cpp
    renderer/scripting/krscriptsection.cpp
    renderer/scripting/krscriptline.cpp

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

    plugins/label/KoReportItemLabel.cpp
    plugins/label/KoReportDesignerItemLabel.cpp
    plugins/label/krscriptlabel.cpp
    plugins/label/KoReportLabelPlugin.cpp

    plugins/check/KoReportItemCheck.cpp
    plugins/check/KoReportDesignerItemCheck.cpp
#    plugins/check/krscriptcheck.cpp
    plugins/check/KoReportCheckPlugin.cpp

    plugins/field/KoReportItemField.cpp
    plugins/field/KoReportDesignerItemField.cpp
    plugins/field/krscriptfield.cpp
    plugins/field/KoReportFieldPlugin.cpp

    plugins/image/KoReportItemImage.cpp
    plugins/image/KoReportDesignerItemImage.cpp
    plugins/image/krscriptimage.cpp
    plugins/image/KoReportImagePlugin.cpp

    plugins/shape/KoReportItemShape.cpp
    plugins/shape/KoReportDesignerItemShape.cpp
#    plugins/shape/krscriptshape.cpp
    plugins/shape/KoReportShapePlugin.cpp

    plugins/text/KoReportItemText.cpp
    plugins/text/KoReportDesignerItemText.cpp
    plugins/text/krscripttext.cpp
    plugins/text/KoReportTextPlugin.cpp
    )

kde4_add_ui_files(koreport_LIB_SRCS wrtembed/detailgroupsectiondialog.ui wrtembed/sectioneditor.ui )

kde4_add_library(koreport SHARED ${koreport_LIB_SRCS})

target_link_libraries(koreport
  ${KOPROPERTY_LIBS} ${KDE4_KROSSCORE_LIBS} komain)

target_link_libraries(koreport LINK_INTERFACE_LIBRARIES
  ${KOPROPERTY_LIBS} ${KDE4_KROSSCORE_LIBS} komain)

set_target_properties(koreport PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS koreport  ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES  koreport_itemplugin.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})

