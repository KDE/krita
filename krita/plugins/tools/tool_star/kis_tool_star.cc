/*
 *  kis_tool_star.cc -- part of Krita
 *
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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


#include <math.h>

#include <QPainter>
#include <QSpinBox>
#include <QLayout>
#include <QGridLayout>

#include <kaction.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include <klocale.h>
#include <kdebug.h>
#include <knuminput.h>

#include "kis_undo_adapter.h"
#include "kis_view.h"
#include "kis_painter.h"
#include "kis_int_spinbox.h"
#include "kis_canvas_subject.h"
#include "kis_canvas_controller.h"
#include "KoPointerEvent.h"
#include "KoPointerEvent.h"
#include "KoPointerEvent.h"
#include "kis_paintop_registry.h"
#include "kis_canvas.h"
#include "QPainter"
#include "kis_cursor.h"
#include "kis_int_spinbox.h"

#include "kis_tool_star.h"

KisToolStar::KisToolStar()
    : super(i18n("Star")),
      m_dragging (false),
      m_currentImage (0)
{
    setObjectName("tool_star");
    setCursor(KisCursor::load("tool_star_cursor.png", 6, 6));
    m_innerOuterRatio=40;
    m_vertices=5;
}

KisToolStar::~KisToolStar()
{
}

void KisToolStar::update (KisCanvasSubject *subject)
{
    super::update (subject);
    if (m_subject)
        m_currentImage = m_subject->currentImg ();
}

void KisToolStar::buttonPress(KoPointerEvent *event)
{
    if (m_currentImage && event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragStart = event->pos();
        m_dragEnd = event->pos();
        m_vertices = m_optWidget->verticesSpinBox->value();
        m_innerOuterRatio = m_optWidget->ratioSpinBox->value();
    }
}

void KisToolStar::move(KoPointerEvent *event)
{
    if (m_dragging) {
        // erase old lines on canvas
        draw(m_dragStart, m_dragEnd);
        // move (alt) or resize star
        if (event->modifiers() & Qt::AltModifier) {
            QPointF trans = event->pos() - m_dragEnd;
            m_dragStart += trans;
            m_dragEnd += trans;
        } else {
            m_dragEnd = event->pos();
        }
        // draw new lines on canvas
        draw(m_dragStart, m_dragEnd);
    }
}

void KisToolStar::buttonRelease(KoPointerEvent *event)
{
    if (!m_subject || !m_currentImage)
        return;

    if (m_dragging && event->button() == Qt::LeftButton) {
        // erase old lines on canvas
        draw(m_dragStart, m_dragEnd);
        m_dragging = false;

        if (m_dragStart == m_dragEnd)
            return;

        if (!m_currentImage)
            return;

        if (!m_currentImage->activeDevice())
            return;

        KisPaintDeviceSP device = m_currentImage->activeDevice ();;
        KisPainter painter (device);
        if (m_currentImage->undo()) painter.beginTransaction (i18n("Star"));

        painter.setPaintColor(m_subject->fgColor());
        painter.setBackgroundColor(m_subject->bgColor());
        painter.setFillStyle(fillStyle());
        painter.setBrush(m_subject->currentBrush());
        painter.setPattern(m_subject->currentPattern());
        painter.setOpacity(m_opacity);
        painter.setCompositeOp(m_compositeOp);
        KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp(m_subject->currentPaintOp(), m_subject->currentPaintOpSettings(), &painter);
        painter.setPaintOp(op); // Painter takes ownership

        vQPointF coord = starCoordinates(m_vertices, m_dragStart.x(), m_dragStart.y(), m_dragEnd.x(), m_dragEnd.y());

        painter.paintPolygon(coord);

        device->setDirty( painter.dirtyRect() );
        notifyModified();

        if (m_currentImage->undo()) {
            m_currentImage->undoAdapter()->addCommand(painter.endTransaction());
        }
    }
}

void KisToolStar::draw(const QPointF& start, const QPointF& end )
{
    if (!m_subject || !m_currentImage)
        return;

    KisCanvasController *controller = m_subject->canvasController();
    KisCanvas *canvas = controller->kiscanvas();
    QPainter p (canvas->canvasWidget());
    QPen pen(Qt::SolidLine);

    QPointF startPos;
    QPointF endPos;
    startPos = controller->windowToView(start);
    endPos = controller->windowToView(end);

    //p.setRasterOp(Qt::NotROP);

    vQPointF points = starCoordinates(m_vertices, startPos.x(), startPos.y(), endPos.x(), endPos.y());

    for (int i = 0; i < points.count() - 1; i++) {
        p.drawLine(points[i].floorQPoint(), points[i + 1].floorQPoint());
    }
    p.drawLine(points[points.count() - 1].floorQPoint(), points[0].floorQPoint());

    p.end ();
}

void KisToolStar::setup(KActionCollection *collection)
{
    m_action = collection->action(objectName());

    if (m_action == 0) {
        m_action = new KAction(KIcon("tool_star"),
                               i18n("&Star"),
                               collection,
                               objectName());
        Q_CHECK_PTR(m_action);
        m_action->setShortcut(KShortcut(Qt::Key_Plus, Qt::Key_F9));
        connect(m_action, SIGNAL(triggered()), this, SLOT(activate()));
        m_action->setToolTip(i18n("Draw a star"));
        m_action->setActionGroup(actionGroup());
        m_ownAction = true;
    }
}

vQPointF KisToolStar::starCoordinates(int N, double mx, double my, double x, double y)
{
    double R=0, r=0;
    qint32 n=0;
    double angle;

    vQPointF starCoordinatesArray(2*N);

    // the radius of the outer edges
    R=sqrt((x-mx)*(x-mx)+(y-my)*(y-my));

    // the radius of the inner edges
    r=R*m_innerOuterRatio/100.0;

    // the angle
    angle=-atan2((x-mx),(y-my));

    //set outer edges
    for(n=0;n<N;n++){
        starCoordinatesArray[2*n] = QPointF(mx+R*cos(n * 2.0 * M_PI / N + angle),my+R*sin(n *2.0 * M_PI / N+angle));
    }

    //set inner edges
    for(n=0;n<N;n++){
        starCoordinatesArray[2*n+1] = QPointF(mx+r*cos((n + 0.5) * 2.0 * M_PI / N + angle),my+r*sin((n +0.5) * 2.0 * M_PI / N + angle));
    }

    return starCoordinatesArray;
}

QWidget* KisToolStar::createOptionWidget()
{
    QWidget *widget = super::createOptionWidget(parent);

    m_optWidget = new WdgToolStar(widget);
    Q_CHECK_PTR(m_optWidget);

    m_optWidget->ratioSpinBox->setValue(m_innerOuterRatio);

    QGridLayout *optionLayout = new QGridLayout(widget);
    super::addOptionWidgetLayout(optionLayout);

    optionLayout->addWidget(m_optWidget, 0, 0);

    return widget;
}

#include "kis_tool_star.moc"
