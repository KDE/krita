/*
 *  kis_tool_line.cc - part of Krayon
 *
 *  Copyright (c) 2000 John Califf <jwcaliff@compuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2003 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2009 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2007,2010 Cyrille Berger <cberger@cberger.net>
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

#include "kis_tool_line.h"

#include <cmath>

#include <QLayout>
#include <QWidget>
#include <QPainter>
#include <QPainterPath>

#include <klocale.h>

#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoPointerEvent.h>
#include <KoPathShape.h>
#include <KoShapeController.h>
#include <KoLineBorder.h>

#include <kis_debug.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_paint_information.h>
#include <kis_cursor.h>
#include <kis_painter.h>
#include <kis_paintop_registry.h>
#include <kis_layer.h>
#include <kis_paint_layer.h>

#include <recorder/kis_action_recorder.h>
#include <recorder/kis_recorded_path_paint_action.h>
#include <recorder/kis_node_query_path.h>



#define ENABLE_RECORDING


KisToolLine::KisToolLine(KoCanvasBase * canvas)
        : KisToolPaint(canvas, KisCursor::load("tool_line_cursor.png", 6, 6))
{
    setObjectName("tool_line");


    m_painter = 0;
    currentImage() = 0;
    m_startPos = QPointF(0, 0);
    m_endPos = QPointF(0, 0);
}

KisToolLine::~KisToolLine()
{
}

void KisToolLine::paint(QPainter& gc, const KoViewConverter &converter)
{

    Q_UNUSED(converter);
    if (mode() == KisTool::PAINT_MODE) {
        paintLine(gc, QRect());
    }
}


void KisToolLine::mousePressEvent(KoPointerEvent *event)
{
    if(PRESS_CONDITION(event, KisTool::HOVER_MODE,
                       Qt::LeftButton, Qt::NoModifier)) {

        setMode(KisTool::PAINT_MODE);

        m_startPos = convertToPixelCoord(event);
        m_endPos = m_startPos;
    }
    else {
        KisToolPaint::mousePressEvent(event);
    }
}


void KisToolLine::mouseMoveEvent(KoPointerEvent *event)
{
    if(MOVE_CONDITION(event, KisTool::PAINT_MODE)) {
        // First ensure the old temp line is deleted
        updatePreview();

        QPointF pos = convertToPixelCoord(event);

        if (event->modifiers() == Qt::AltModifier) {
            QPointF trans = pos - m_endPos;
            m_startPos += trans;
            m_endPos += trans;
        } else if (event->modifiers() == Qt::ShiftModifier) {
            m_endPos = straightLine(pos);
        } else {
            m_endPos = pos;
        }
        updatePreview();
    }
    else {
        KisToolPaint::mouseMoveEvent(event);
    }
}

void KisToolLine::mouseReleaseEvent(KoPointerEvent *event)
{
    if(RELEASE_CONDITION(event, KisTool::PAINT_MODE, Qt::LeftButton)) {
        setMode(KisTool::HOVER_MODE);

        updatePreview();

        QPointF pos = convertToPixelCoord(event);

        if (event->modifiers() == Qt::AltModifier) {
            QPointF trans = pos - m_endPos;
            m_startPos += trans;
            m_endPos += trans;
        } else if (event->modifiers() == Qt::ShiftModifier) {
            m_endPos = straightLine(pos);
        } else {
            m_endPos = pos;
        }

        if (m_startPos == m_endPos) {
            return;
        }

#ifdef ENABLE_RECORDING
        if (image()) {
            KisRecordedPathPaintAction linePaintAction(KisNodeQueryPath::absolutePath(currentNode()), currentPaintOpPreset());
            setupPaintAction(&linePaintAction);
            linePaintAction.addLine(m_startPos, m_endPos);
            image()->actionRecorder()->addAction(linePaintAction);
        }
#endif
        NodePaintAbility nodeAbility = nodePaintAbility();
        if (nodeAbility == NONE) {
           return;
        }

        if (nodeAbility == PAINT) {
                KisPaintDeviceSP device = currentNode()->paintDevice();
                delete m_painter;
                m_painter = new KisPainter(device, currentSelection());
                Q_CHECK_PTR(m_painter);

                m_painter->beginTransaction(i18nc("a straight drawn line", "Line"));
                setupPainter(m_painter);
                m_painter->paintLine(m_startPos, m_endPos);

                QRegion dirtyRegion = m_painter->takeDirtyRegion();
                m_painter->endTransaction(image()->undoAdapter());

                device->setDirty(dirtyRegion);
                notifyModified();

                delete m_painter;
                m_painter = 0;
        }
        else {
            KoPathShape* path = new KoPathShape();
            path->setShapeId(KoPathShapeId);

            QTransform resolutionMatrix;
            resolutionMatrix.scale(1 / currentImage()->xRes(), 1 / currentImage()->yRes());
            path->moveTo(resolutionMatrix.map(m_startPos));
            path->lineTo(resolutionMatrix.map(m_endPos));
            path->normalize();

            KoLineBorder* border = new KoLineBorder(1.0, currentFgColor().toQColor());
            path->setBorder(border);

            QUndoCommand * cmd = canvas()->shapeController()->addShape(path);
            canvas()->addCommand(cmd);
        }
    }
    else {
        KisToolPaint::mouseReleaseEvent(event);
    }
}


QPointF KisToolLine::straightLine(QPointF point)
{
    const QPointF lineVector = point - m_startPos;
    qreal lineAngle = std::atan2(lineVector.y(), lineVector.x());

    if (lineAngle < 0) {
        lineAngle += 2 * M_PI;
    }

    const qreal ANGLE_BETWEEN_CONSTRAINED_LINES = (2 * M_PI) / 24;

    const quint32 constrainedLineIndex = static_cast<quint32>((lineAngle / ANGLE_BETWEEN_CONSTRAINED_LINES) + 0.5);
    const qreal constrainedLineAngle = constrainedLineIndex * ANGLE_BETWEEN_CONSTRAINED_LINES;

    const qreal lineLength = std::sqrt((lineVector.x() * lineVector.x()) + (lineVector.y() * lineVector.y()));

    const QPointF constrainedLineVector(lineLength * std::cos(constrainedLineAngle), lineLength * std::sin(constrainedLineAngle));

    const QPointF result = m_startPos + constrainedLineVector;

    return result;
}


void KisToolLine::updatePreview()
{
    if (canvas()) {
        QRectF bound(m_startPos, m_endPos);
        canvas()->updateCanvas(convertToPt(bound.normalized().adjusted(-3, -3, 3, 3)));
    }
}


void KisToolLine::paintLine(QPainter& gc, const QRect&)
{
    QPointF viewStartPos = pixelToView(m_startPos);
    QPointF viewStartEnd = pixelToView(m_endPos);

    if (canvas()) {
        QPainterPath path;
        path.moveTo(viewStartPos);
        path.lineTo(viewStartEnd);
        paintToolOutline(&gc, path);
    }
}

QString KisToolLine::quickHelp() const
{
    return i18n("Alt+Drag will move the origin of the currently displayed line around, Shift+Drag will force you to draw straight lines");
}

#include "kis_tool_line.moc"

