/*
 *  kis_tool_select_brush.cc -- part of Krita
 *
 *  Copyright (C) 2010 Celarek Adam <kdedev at xibo dot at>
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

#include <Eigen/Core>
USING_PART_OF_NAMESPACE_EIGEN

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
        : KisToolSelectBase(canvas,
                            KisCursor::load("tool_brush_selection_cursor.png", 6, 6),
                            i18n("Brush Selection")),
        m_brushRadius(15),
        m_lastMousePosition(-1, -1)
{
    resetSelection();
}

KisToolSelectBrush::~KisToolSelectBrush()
{
}

QWidget* KisToolSelectBrush::createOptionWidget()
{
    KisToolSelectBase::createOptionWidget();
    KisSelectionOptions *selectionWidget = selectionOptionWidget();

    QHBoxLayout* fl = new QHBoxLayout();
    QLabel * lbl = new QLabel(i18n("Brush size:"), selectionWidget);
    fl->addWidget(lbl);

    KIntNumInput * input = new KIntNumInput(selectionWidget);
    input->setRange(0, 500, 5);
    input->setValue(m_brushRadius*2);
    fl->addWidget(input);
    connect(input, SIGNAL(valueChanged(int)), this, SLOT(slotSetBrushSize(int)));

    QVBoxLayout* l = dynamic_cast<QVBoxLayout*>(selectionWidget->layout());
    Q_ASSERT(l);
    l->insertLayout(1, fl);

    selectionWidget->disableSelectionModeOption();

    return selectionWidget;
}

void KisToolSelectBrush::paint(QPainter& gc, const KoViewConverter &converter)
{
    Q_UNUSED(converter);
    if(mode() == KisTool::PAINT_MODE) {
        paintToolOutline(&gc, pixelToView(m_selection));
    }
    else  if(m_lastMousePosition!=QPoint(-1, -1)) {
        QPainterPath brushOutline;
        brushOutline.addEllipse(m_lastMousePosition, m_brushRadius, m_brushRadius);
        paintToolOutline(&gc, pixelToView(brushOutline));
    }
}

void KisToolSelectBrush::mousePressEvent(KoPointerEvent *event)
{
    if(mode() == KisTool::PAINT_MODE) {
        /**
         * Cancalling must be done at KisTool level
         */
//        resetSelection();
//        return;
    }

    if(PRESS_CONDITION(event, KisTool::HOVER_MODE,
                       Qt::LeftButton, Qt::NoModifier)) {

        if (!currentNode()) {
            return;
        }

        if (!selectionEditable()) {
            return;
        }

        setMode(KisTool::PAINT_MODE);

        m_lastPoint = convertToPixelCoord(event->point);
        addPoint(m_lastPoint);
    }
    else {
        KisTool::mousePressEvent(event);
    }
}

void KisToolSelectBrush::mouseMoveEvent(KoPointerEvent *event)
{
    /**
     * Update outline
     */
    QRect brushRect(-m_brushRadius, -m_brushRadius, 2*m_brushRadius, 2*m_brushRadius);
    brushRect.adjust(-2, -2, 2, 2);     //width of tool outline

    brushRect.moveCenter(m_lastMousePosition);
    updateCanvasPixelRect(brushRect);

    m_lastMousePosition = convertToPixelCoord(event).toPoint();

    brushRect.moveCenter(m_lastMousePosition);
    updateCanvasPixelRect(brushRect);

    /**
     * Do selection
     */
    if(MOVE_CONDITION(event, KisTool::PAINT_MODE)) {
        // this gives better performance
        if(Vector2f((m_lastPoint-convertToPixelCoord(event->point)).x(), (m_lastPoint-convertToPixelCoord(event->point)).y()).norm()<m_brushRadius/6)
            return;

        //randomise the point to workaround a bug in QPainterPath::operator|=()
        //FIXME: http://bugreports.qt.nokia.com/browse/QTBUG-8035
        //will be fixed in version 4.7.0
        qreal randomX=rand()%100;
        randomX/=1000.;
        qreal randomY=rand()%100;
        randomY/=1000.;
        QPointF smallRandomPoint(randomX, randomY);
        addPoint(convertToPixelCoord(event->point)+smallRandomPoint);
    }
    else {
        KisTool::mouseMoveEvent(event);
    }
}

void KisToolSelectBrush::mouseReleaseEvent(KoPointerEvent *event)
{
    if(RELEASE_CONDITION(event, KisTool::PAINT_MODE, Qt::LeftButton)) {
        setMode(KisTool::HOVER_MODE);
        applyToSelection(m_selection);
    }
    else {
        KisTool::mouseReleaseEvent(event);
    }
}

void KisToolSelectBrush::deactivate()
{
    resetSelection();
    KisToolSelectBase::deactivate();
}

void KisToolSelectBrush::slotSetBrushSize(int size)
{
    m_brushRadius = ((qreal) size)/2.0;
}

void KisToolSelectBrush::applyToSelection(const QPainterPath &selection) {
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    Q_ASSERT(kisCanvas);
    if (!kisCanvas)
        return;

    KisSelectionToolHelper helper(kisCanvas, currentNode(), i18n("Brush Selection"));

    if (selectionMode() == PIXEL_SELECTION) {

        KisPixelSelectionSP tmpSel = new KisPixelSelection();

        KisPainter painter(tmpSel);
        painter.setBounds(currentImage()->bounds());
        painter.setPaintColor(KoColor(Qt::black, tmpSel->colorSpace()));
        painter.setPaintOpPreset(currentPaintOpPreset(), currentImage());
        painter.setAntiAliasPolygonFill(selectionOptionWidget()->antiAliasSelection());
        painter.setFillStyle(KisPainter::FillStyleForegroundColor);
        painter.setStrokeStyle(KisPainter::StrokeStyleNone);

        painter.fillPainterPath(selection);

        helper.selectPixelSelection(tmpSel, selectionAction());

        resetSelection();
    }
}

void KisToolSelectBrush::resetSelection()
{
    updateCanvasPixelRect(m_selection.boundingRect());
    m_selection = QPainterPath();
}

void KisToolSelectBrush::addPoint(const QPointF& point)
{
    QPainterPath ellipse;
    ellipse.addEllipse(point, m_brushRadius, m_brushRadius);

    m_selection |= (ellipse);
    addGap(m_lastPoint, point);

    updateCanvasPixelRect(QRectF(m_lastPoint, point).normalized().adjusted(-m_brushRadius, -m_brushRadius, m_brushRadius, m_brushRadius));

    m_lastPoint = point;
}


void KisToolSelectBrush::addGap(const QPointF& start, const QPointF& end)
{
    Vector2f way((end-start).x(), (end-start).y());
    if(way.norm() < m_brushRadius/3.)
        return;

    Vector2f direction(way.normalized());

    //rotate 90 degrees clockwise
    Vector2f rotatedPlus(direction.y(), -direction.x());

    //rotate 90 degrees counter clockwise
    Vector2f rotatedMinus(-direction.y(), direction.x());

    Vector2f p1(rotatedPlus * m_brushRadius);
    Vector2f p2(way+p1);
    Vector2f p4(rotatedMinus * m_brushRadius);
    Vector2f p3(way+p4);

    //we need to convert floating point vectors to int ones because of a bug in QPainterPath::operator|=()
    //FIXME: http://bugreports.qt.nokia.com/browse/QTBUG-8035
    //converting int to float should be done with rounding, so don't use eigen casting.
    //parameter start contains important decimal places, these shouldn't be lost.
    QPointF pp1 = QPointF(p1.x(), p1.y()).toPoint();
    QPointF pp2 = QPointF(p2.x(), p2.y()).toPoint();
    QPointF pp3 = QPointF(p3.x(), p3.y()).toPoint();
    QPointF pp4 = QPointF(p4.x(), p4.y()).toPoint();

    pp1+=start;
    pp2+=start;
    pp3+=start;
    pp4+=start;

    QPainterPath gap;
    gap.moveTo(pp1);
    gap.lineTo(pp2);
    gap.lineTo(pp3);
    gap.lineTo(pp4);
    gap.closeSubpath();

    m_selection |= (gap);
}


#include "kis_tool_select_brush.moc"
