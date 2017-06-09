/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

    DefaultToolGeometryWidget *geometryWidget = new DefaultToolGeometryWidget(tool, this);
    geometryWidget->setWindowTitle(i18n("Geometry"));
    addTab(geometryWidget, KisIconUtils::loadIcon("geometry"), QString());

    m_strokeWidget = new KoStrokeConfigWidget(tool->canvas(), this);
    m_strokeWidget->setWindowTitle(i18n("Stroke"));

    KisDocumentAwareSpinBoxUnitManager* managerLineWidth = new KisDocumentAwareSpinBoxUnitManager(m_strokeWidget);
    KisDocumentAwareSpinBoxUnitManager* managerMitterLimit = new KisDocumentAwareSpinBoxUnitManager(m_strokeWidget);
    managerLineWidth->setApparentUnitFromSymbol("px");
    managerMitterLimit->setApparentUnitFromSymbol("px"); //set unit to px by default
    m_strokeWidget->setUnitManagers(managerLineWidth, managerMitterLimit);

    addTab(m_strokeWidget, KisIconUtils::loadIcon("krita_tool_line"), QString());


    m_fillWidget = new KoFillConfigWidget(tool->canvas(), KoFlake::Fill, this);
    m_fillWidget->setWindowTitle(i18n("Fill"));
    addTab(m_fillWidget, KisIconUtils::loadIcon("krita_tool_color_fill"), QString());

    connect(this, SIGNAL(currentChanged(int)), SLOT(slotCurrentIndexChanged(int)));
    m_oldTabIndex = currentIndex();
}

DefaultToolTabbedWidget::~DefaultToolTabbedWidget()
{
}

void DefaultToolTabbedWidget::activate()
{
    m_fillWidget->activate();
    m_strokeWidget->activate();
}

void DefaultToolTabbedWidget::deactivate()
{
    m_fillWidget->deactivate();
    m_strokeWidget->deactivate();
}

void DefaultToolTabbedWidget::slotCurrentIndexChanged(int current)
{
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
