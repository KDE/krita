#make it a project so cmake spits out a codeblocks project
#project(reports)

include_directories( ${KOMAIN_INCLUDES} 
${KOODF_INCLUDES}
${CMAKE_SOURCE_DIR}/libs/
${CMAKE_SOURCE_DIR}/libs/koreport
${CMAKE_SOURCE_DIR}/libs/koreport/common
${CMAKE_SOURCE_DIR}/libs/koreport/renderer
${CMAKE_SOURCE_DIR}/kchart/kdchart/include

)

#build a shared library
set(koreport_LIB_SRCS
	common/krutils.cpp
	common/krpos.cpp
	common/krsize.cpp
	common/krobjectdata.cpp
	common/krchartdata.cpp
	common/krlabeldata.cpp
	common/krfielddata.cpp
	common/krtextdata.cpp
	common/krbarcodedata.cpp
	common/krimagedata.cpp
	common/krlinedata.cpp
	common/krshapedata.cpp
	common/krcheckdata.cpp
	common/krsectiondata.cpp
	common/labelsizeinfo.cpp
	common/reportpageoptions.cpp
	common/krreportdata.cpp
	common/krdetailsectiondata.cpp
	
	renderer/3of9.cpp
	renderer/code128.cpp
	renderer/codeean.cpp
	renderer/ext3of9.cpp
	renderer/renderobjects.cpp
	renderer/KoReportPrintRenderer.cpp
	renderer/KoReportPreRenderer.cpp
	renderer/KoReportScreenRenderer.cpp
	renderer/KoReportHTMLTableRenderer.cpp
    renderer/KoReportHTMLCSSRenderer.cpp
    renderer/KoReportKSpreadRenderer.cpp
    renderer/KoReportRendererBase.cpp

	renderer/scripting/krscripthandler.cpp
	renderer/scripting/krscriptconstants.cpp
	renderer/scripting/krscriptdebug.cpp
	renderer/scripting/krscriptdraw.cpp
	renderer/scripting/krscriptreport.cpp
	renderer/scripting/krscriptsection.cpp
	renderer/scripting/krscriptlabel.cpp
	renderer/scripting/krscriptfield.cpp
	renderer/scripting/krscripttext.cpp
	renderer/scripting/krscriptbarcode.cpp
	renderer/scripting/krscriptline.cpp
	renderer/scripting/krscriptimage.cpp
	renderer/scripting/krscriptchart.cpp

	renderer/ods/KoSimpleOdsDocument.cpp
	renderer/ods/KoSimpleOdsSheet.cpp
	renderer/ods/KoSimpleOdsCell.cpp

	wrtembed/detailgroupsectiondialog.cpp
	wrtembed/reportentities.cpp
	wrtembed/reportrectentity.cpp
	wrtembed/reportentitylabel.cpp
	wrtembed/reportentityfield.cpp
	wrtembed/reportentitytext.cpp
	wrtembed/reportentityline.cpp
	wrtembed/reportentitybarcode.cpp
	wrtembed/reportentityimage.cpp
	wrtembed/reportentitychart.cpp
	wrtembed/reportentityshape.cpp
	wrtembed/reportentitycheck.cpp
	wrtembed/reportsection.cpp
	wrtembed/reportsectiondetailgroup.cpp
	wrtembed/reportsectiondetail.cpp
	wrtembed/reportscene.cpp
	wrtembed/reportsceneview.cpp
	wrtembed/KoReportDesigner.cpp
	wrtembed/reportpropertiesbutton.cpp
	wrtembed/sectioneditor.cpp
	wrtembed/3of9paint.cpp
	wrtembed/ext3of9paint.cpp
	wrtembed/codeeanpaint.cpp
	wrtembed/code128paint.cpp
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
