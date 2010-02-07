/*
 *  kis_tool_select_brush.cc -- part of Krita
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

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QPainterPath>


#include <KIntNumInput>

#include <KoCanvasBase.h>
#include <KoViewConverter.h>
#include <KoPointerEvent.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>

#include "kis_cursor.h"
#include "kis_canvas2.h"
#include "kis_painter.h"
#include "kis_pixel_selection.h"
#include "kis_image.h"
#include "kis_selection_options.h"
#include "kis_tool_select_brush.h"
#include "kis_selection_tool_helper.h"
#include "kis_paintop_preset.h"


KisToolSelectBrush::KisToolSelectBrush(KoCanvasBase * canvas)
        : KisToolSelectBase(canvas, KisCursor::load("tool_brush_selection_cursor.png", 6, 6)),
        m_brusRadius(15),
        m_dragging(false),
        m_selection(new QPainterPath())
{
}

KisToolSelectBrush::~KisToolSelectBrush()
{
    delete m_selection;
}

QWidget* KisToolSelectBrush::createOptionWidget()
{
    KisToolSelectBase::createOptionWidget();
    m_optWidget->setWindowTitle(i18n("Brush Selection"));

    QHBoxLayout* fl = new QHBoxLayout();
    QLabel * lbl = new QLabel(i18n("Brush size:"), m_optWidget);
    fl->addWidget(lbl);

    KIntNumInput * input = new KIntNumInput(m_optWidget);
    input->setRange(0, 500, 5);
    input->setValue(m_brusRadius);
    fl->addWidget(input);
    connect(input, SIGNAL(valueChanged(int)), this, SLOT(slotSetBrushSize(int)));

    QVBoxLayout* l = dynamic_cast<QVBoxLayout*>(m_optWidget->layout());
    Q_ASSERT(l);
    l->insertLayout(1, fl);

    m_optWidget->disableSelectionModeOption();

    return m_optWidget;
}

void KisToolSelectBrush::paint(QPainter& gc, const KoViewConverter &converter)
{
    gc.save();
    qreal sx, sy;
    converter.zoom(&sx, &sy);
    gc.scale(sx / image()->xRes(), sy / image()->xRes());
    paintToolOutline(&gc, *m_selection);
    gc.restore();
}

void KisToolSelectBrush::mousePressEvent(KoPointerEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_dragging = true;
        addPoint(e->point);
        e->accept();
    }
    else if (e->button()==Qt::RightButton || e->button()==Qt::MidButton) {
        m_dragging = false;
        resetSelection();
        e->accept();
    }
}

void KisToolSelectBrush::mouseMoveEvent(KoPointerEvent *e)
{
    if (m_dragging) {
        addPoint(e->point);
    }
}

void KisToolSelectBrush::mouseReleaseEvent(KoPointerEvent *e)
{
    if (m_dragging && e->button() == Qt::LeftButton) {
        m_dragging = false;
        applyToSelection(*m_selection);
        e->accept();
    }
}

void KisToolSelectBrush::activate(bool temprorary)
{
    KisToolSelectBase::activate(temprorary);

}

void KisToolSelectBrush::deactivate()
{
    resetSelection();
    KisToolSelectBase::deactivate();
}

void KisToolSelectBrush::slotSetBrushSize(int size)
{
    m_brusRadius = ((qreal) size)/2.0;
}

void KisToolSelectBrush::applyToSelection(const QPainterPath &selection) {
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    Q_ASSERT(kisCanvas);
    if (!kisCanvas)
        return;

    KisSelectionToolHelper helper(kisCanvas, currentNode(), i18n("Brush Selection"));

    if (m_selectionMode == PIXEL_SELECTION) {

        KisPixelSelectionSP tmpSel = new KisPixelSelection();

        KisPainter painter(tmpSel);
        painter.setBounds(currentImage()->bounds());
        painter.setPaintColor(KoColor(Qt::black, tmpSel->colorSpace()));
        painter.setFillStyle(KisPainter::FillStyleForegroundColor);
        painter.setStrokeStyle(KisPainter::StrokeStyleNone);
        painter.setAntiAliasPolygonFill(m_optWidget->antiAliasSelection());
        painter.setOpacity(OPACITY_OPAQUE);
        painter.setPaintOpPreset(currentPaintOpPreset(), currentImage());
        painter.setCompositeOp(tmpSel->colorSpace()->compositeOp(COMPOSITE_OVER));

//        painter.paintPainterPath(selection);
        painter.fillPainterPath(selection);

        QUndoCommand* cmd = helper.selectPixelSelection(tmpSel, m_selectAction);
        canvas()->addCommand(cmd);

        resetSelection();
    }
}

void KisToolSelectBrush::resetSelection()
{
    updateCanvasPixelRect(m_selection->boundingRect());
    m_dragging=false;
    delete m_selection;
    m_selection = new QPainterPath();
}

void KisToolSelectBrush::addPoint(const QPointF& point)
{
    QPainterPath ellipse;
    ellipse.addEllipse(convertToPixelCoord(point), m_brusRadius, m_brusRadius);
    m_selection->operator|=(ellipse);

    QPointF tmp = QPointF(m_brusRadius, m_brusRadius);
    updateCanvasPixelRect(QRectF(convertToPixelCoord(point)-tmp, convertToPixelCoord(point)+tmp));
}


#include "kis_tool_select_brush.moc"
