/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "DefaultToolTabbedWidget.h"

#include <QLabel>
#include "kis_icon_utils.h"
#include "DefaultToolGeometryWidget.h"
#include "KoStrokeConfigWidget.h"
#include "KoFillConfigWidget.h"
#include <KoInteractionTool.h>

#include <kis_document_aware_spin_box_unit_manager.h>


DefaultToolTabbedWidget::DefaultToolTabbedWidget(KoInteractionTool *tool, QWidget *parent)
    : KoTitledTabWidget(parent)
{    
    setObjectName("default-tool-tabbed-widget");

    m_geometryWidget = new DefaultToolGeometryWidget(tool, this);
    m_geometryWidget->setWindowTitle(i18n("Geometry"));
    addTab(m_geometryWidget, KisIconUtils::loadIcon("geometry"), QString());

    m_strokeWidget = new KoStrokeConfigWidget(tool->canvas(), this);
    m_strokeWidget->setWindowTitle(i18nc("Draws a line around an area", "Stroke"));

    KisDocumentAwareSpinBoxUnitManager* managerLineWidth = new KisDocumentAwareSpinBoxUnitManager(m_strokeWidget);
    KisDocumentAwareSpinBoxUnitManager* managerMitterLimit = new KisDocumentAwareSpinBoxUnitManager(m_strokeWidget);
    managerLineWidth->setApparentUnitFromSymbol("px");
    managerMitterLimit->setApparentUnitFromSymbol("px"); //set unit to px by default
    m_strokeWidget->setUnitManagers(managerLineWidth, managerMitterLimit);

    addTab(m_strokeWidget, KisIconUtils::loadIcon("krita_tool_line"), QString());


    m_fillWidget = new KoFillConfigWidget(tool->canvas(), KoFlake::Fill, true, this);
    m_fillWidget->setWindowTitle(i18n("Fill"));
    addTab(m_fillWidget, KisIconUtils::loadIcon("krita_tool_color_fill"), QString());

    connect(this, SIGNAL(currentChanged(int)), SLOT(slotCurrentIndexChanged(int)));
    m_oldTabIndex = currentIndex();

    connect(m_fillWidget, SIGNAL(sigMeshGradientResetted()), SIGNAL(sigMeshGradientResetted()));
}

DefaultToolTabbedWidget::~DefaultToolTabbedWidget()
{
}

void DefaultToolTabbedWidget::activate()
{
    if (currentIndex() == StrokeTab) {
        m_strokeWidget->activate();
    } else {
        m_fillWidget->activate();
    }
}

void DefaultToolTabbedWidget::deactivate()
{
    if (m_oldTabIndex == StrokeTab) {
        m_strokeWidget->deactivate();
    } else {
        m_fillWidget->deactivate();
    }
}

bool DefaultToolTabbedWidget::useUniformScaling() const
{
    return m_geometryWidget->useUniformScaling();
}

void DefaultToolTabbedWidget::slotMeshGradientHandleSelected(KoShapeMeshGradientHandles::Handle h)
{
    if (h.type == KoShapeMeshGradientHandles::Handle::Corner) {
        m_fillWidget->setSelectedMeshGradientHandle(SvgMeshPosition {h.row, h.col, h.segmentType});
    } else {
        m_fillWidget->setSelectedMeshGradientHandle(SvgMeshPosition());
    }
}

void DefaultToolTabbedWidget::slotCurrentIndexChanged(int current)
{
    // because of nesting we are required to only let one widget be active at at time
    deactivate();
    activate();

    if (m_oldTabIndex == FillTab) {
        emit sigSwitchModeEditFillGradient(false);
    } else if (m_oldTabIndex == StrokeTab) {
        emit sigSwitchModeEditStrokeGradient(false);
    }

    m_oldTabIndex = current;

    if (current == FillTab) {
        emit sigSwitchModeEditFillGradient(true);
    } else if (m_oldTabIndex == StrokeTab) {
        emit sigSwitchModeEditStrokeGradient(true);
    }
}
