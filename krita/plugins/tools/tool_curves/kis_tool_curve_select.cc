/*
 *  kis_tool_curve_select.cc -- part of Krita
 *
 *  Copyright (c) 2006 Emanuele Tamponi <emanuele@valinor.it>
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

#include <qapplication.h>
#include <qpainter.h>
#include <qpen.h>
#include <qlayout.h>

#include <kdebug.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>

#include "kis_autobrush_resource.h"
#include "kis_canvas_controller.h"
#include "kis_canvas_subject.h"
#include "kis_cursor.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_layer.h"
#include "kis_undo_adapter.h"
#include "kis_selection.h"
#include "kis_selection_options.h"
#include "kis_selected_transaction.h"
#include "kis_canvas.h"
#include "kis_canvas_painter.h"
#include "kis_paintop_registry.h"

#include "kis_tool_curve_select.h"

KisToolCurveSelect::KisToolCurveSelect (const QString& UIName)
    : super(UIName)
{
    m_UIName = UIName;
    m_optWidget = 0;
    m_selectAction = SELECTION_ADD;
}

KisToolCurveSelect::~KisToolCurveSelect ()
{

}

QValueVector<KisPoint> KisToolCurveSelect::convertCurve()
{
    QValueVector<KisPoint> points;

    for (KisCurve::iterator i = m_curve->begin(); i != m_curve->end(); i++)
        points.append((*i).point());

    return points;
}

void KisToolCurveSelect::paintCurve()
{
    QApplication::setOverrideCursor(KisCursor::waitCursor());
    KisPaintDeviceSP dev = m_currentImage->activeDevice();
    bool hasSelection = dev->hasSelection();
    KisSelectedTransaction *t = 0;
    if (m_currentImage->undo()) t = new KisSelectedTransaction(i18n(m_transactionMessage.ascii()), dev);
    KisSelectionSP selection = dev->selection();

    if (!hasSelection) {
        selection->clear();
    }

    KisPainter painter(selection.data());

    painter.setPaintColor(KisColor(Qt::black, selection->colorSpace()));
    painter.setFillStyle(KisPainter::FillStyleForegroundColor);
    painter.setStrokeStyle(KisPainter::StrokeStyleNone);
    painter.setBrush(m_subject->currentBrush());
    painter.setOpacity(OPACITY_OPAQUE);
    KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp("paintbrush", 0, &painter);
    painter.setPaintOp(op);    // And now the painter owns the op and will destroy it.

    switch (m_selectAction) {
    case SELECTION_ADD:
        painter.setCompositeOp(COMPOSITE_OVER);
        break;
    case SELECTION_SUBTRACT:
        painter.setCompositeOp(COMPOSITE_SUBTRACT);
        break;
    default:
        break;
    }

    painter.paintPolygon(convertCurve());


    if(hasSelection) {
        QRect dirty(painter.dirtyRect());
        dev->setDirty(dirty);
        dev->emitSelectionChanged(dirty);
    } else {
        dev->setDirty();
        dev->emitSelectionChanged();
    }

    if (m_currentImage->undo())
        m_currentImage->undoAdapter()->addCommand(t);

    QApplication::restoreOverrideCursor();

    draw();
}

void KisToolCurveSelect::slotSetAction(int action) {
    if (action >= SELECTION_ADD && action <= SELECTION_SUBTRACT)
        m_selectAction =(enumSelectionMode)action;
}

QWidget* KisToolCurveSelect::createOptionWidget(QWidget* parent)
{
    m_optWidget = new KisSelectionOptions(parent, m_subject);
    Q_CHECK_PTR(m_optWidget);
    m_optWidget->setCaption(i18n(m_UIName.ascii()));

    connect (m_optWidget, SIGNAL(actionChanged(int)), this, SLOT(slotSetAction(int)));

    QVBoxLayout * l = dynamic_cast<QVBoxLayout*>(m_optWidget->layout());
    l->addItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));

    return m_optWidget;
}

QWidget* KisToolCurveSelect::optionWidget()
{
        return m_optWidget;
}

#include "kis_tool_curve_select.moc"
