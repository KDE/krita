/*
 *  Copyright (c) 2005 Adrian Page
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

#include <qwidget.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qlabel.h>
//Added by qt3to4:
#include <Q3GridLayout>

#include <kdebug.h>
#include <klocale.h>

#include "kis_tool_shape.h"
#include "wdgshapeoptions.h"

KisToolShape::KisToolShape(const QString& UIName) : super(UIName)
{
    m_shapeOptionsWidget = 0;
    m_optionLayout = 0;
}

KisToolShape::~KisToolShape()
{
}

QWidget* KisToolShape::createOptionWidget(QWidget* parent)
{
    QWidget *widget = super::createOptionWidget(parent);

    m_shapeOptionsWidget = new WdgGeometryOptions(0);
    Q_CHECK_PTR(m_shapeOptionsWidget);

    m_optionLayout = new Q3GridLayout(widget, 2, 1);
   // super::addOptionWidgetLayout(m_optionLayout);

    m_shapeOptionsWidget->cmbFill->reparent(widget, QPoint(0,0), true);
    m_shapeOptionsWidget->textLabel3->reparent(widget, QPoint(0,0), true);
    addOptionWidgetOption(m_shapeOptionsWidget->cmbFill, m_shapeOptionsWidget->textLabel3);

    return widget;
}

KisPainter::FillStyle KisToolShape::fillStyle(void)
{
    if (m_shapeOptionsWidget) {
        return static_cast<KisPainter::FillStyle>(m_shapeOptionsWidget->cmbFill->currentItem());
    } else {
        return KisPainter::FillStyleNone;
    }
}

#include "kis_tool_shape.moc"

