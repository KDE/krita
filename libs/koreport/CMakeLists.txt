#make it a project so cmake spits out a codeblocks project
#project(reports)

include_directories( ${KOMAIN_INCLUDES} 
${KOODF_INCLUDES}
${CMAKE_SOURCE_DIR}/libs/
${CMAKE_SOURCE_DIR}/libs/koreport
${CMAKE_SOURCE_DIR}/libs/koreport/common
${CMAKE_SOURCE_DIR}/libs/koreport/renderer
${CMAKE_SOURCE_DIR}/libs/koreport/wrtembed
${CMAKE_SOURCE_DIR}/kchart/kdchart/include

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

    plugins/barcode/KoReportItemBarcode.cpp
    plugins/barcode/KoReportDesignerItemBarcode.cpp
    plugins/barcode/krscriptbarcode.cpp
    plugins/barcode/3of9.cpp
    plugins/barcode/code128.cpp
    plugins/barcode/codeean.cpp
    plugins/barcode/ext3of9.cpp
    plugins/barcode/3of9paint.cpp
    plugins/barcode/ext3of9paint.cpp
    plugins/barcode/codeeanpaint.cpp
    plugins/barcode/code128paint.cpp
    plugins/barcode/KoReportBarcodePlugin.cpp

    plugins/chart/KoReportItemChart.cpp
    plugins/chart/KoReportDesignerItemChart.cpp
    plugins/chart/krscriptchart.cpp
    plugins/chart/KoReportChartPlugin.cpp

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

    plugins/label/KoReportItemLabel.cpp
    plugins/label/KoReportDesignerItemLabel.cpp
    plugins/label/krscriptlabel.cpp
    plugins/label/KoReportLabelPlugin.cpp

    plugins/shape/KoReportItemShape.cpp
    plugins/shape/KoReportDesignerItemShape.cpp
#    plugins/shape/krscriptshape.cpp
    plugins/shape/KoReportShapePlugin.cpp

    plugins/text/KoReportItemText.cpp
    plugins/text/KoReportDesignerItemText.cpp
    plugins/text/krscripttext.cpp
    plugins/text/KoReportTextPlugin.cpp
	)

if (SHOULD_BUILD_KWORD)
  add_definitions(-DHAVE_KWORD)
endif (SHOULD_BUILD_KWORD)

kde4_add_ui_files(koreport_LIB_SRCS wrtembed/detailgroupsectiondialog.ui wrtembed/sectioneditor.ui )

kde4_add_library(koreport SHARED ${koreport_LIB_SRCS})

target_link_libraries(koreport ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY}
  ${KOPROPERTY_LIBS} kdchart ${KDE4_KROSSCORE_LIBS} komain ${QT_QT3SUPPORT_LIBRARY})
if (SHOULD_BUILD_KSPREAD)
    target_link_libraries(koreport kspreadcommon)
endif (SHOULD_BUILD_KSPREAD)

set_target_properties(koreport PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS koreport  ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(pics)
