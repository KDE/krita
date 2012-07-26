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


#include <QPushButton>

#include <KoCanvasBase.h>
#include <KoPointerEvent.h>
#include <KoPathShape.h>
#include <KoShapeController.h>
#include <KoShapeStroke.h>

#include <kis_debug.h>
#include <kis_cursor.h>
#include <kis_paintop_registry.h>
#include "kis_figure_painting_tool_helper.h"

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
}

KisToolLine::~KisToolLine()
{
}

int KisToolLine::flags() const
{
    return KisTool::FLAG_USES_CUSTOM_COMPOSITEOP|KisTool::FLAG_USES_CUSTOM_PRESET;
}

QWidget* KisToolLine::createOptionWidget()
{
    QWidget* widget = KisToolPaint::createOptionWidget();
    m_cbPressure     = new QCheckBox(i18n("Pressure"));
    m_cbTilt         = new QCheckBox(i18n("Tilt"));
    m_cbRotation     = new QCheckBox(i18n("Rotation"));
    m_cbTangPressure = new QCheckBox(i18n("Tangential Pressure"));
    m_bnVaryingEnds  = new QPushButton(i18n("Varying End-Points"));

    m_cbPressure->setChecked(true);
    m_cbTilt->setChecked(true);
    m_cbRotation->setChecked(true);
    m_cbTangPressure->setChecked(true);
    m_bnVaryingEnds->setCheckable(true);

    addOptionWidgetOption(m_cbPressure);
    addOptionWidgetOption(m_cbTilt);
    addOptionWidgetOption(m_cbRotation);
    addOptionWidgetOption(m_cbTangPressure);
    addOptionWidgetOption(m_bnVaryingEnds);
    return widget;
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

        if (nodePaintAbility() == NONE) {
           return;
        }

        if (!nodeEditable()) {
            return;
        }

        setMode(KisTool::PAINT_MODE);

        m_startPos = KisPaintInformation(
            convertToPixelCoord(event),
            PRESSURE_DEFAULT,
            m_cbTilt->isChecked() ? event->xTilt() : 0.0,
            m_cbTilt->isChecked() ? event->yTilt() : 0.0,
            nullKisVector2D(),
            m_cbRotation->isChecked() ? event->rotation() : 0.0,
            m_cbTangPressure->isChecked() ? event->tangentialPressure() : 0.0
        );

        m_endPos      = m_startPos;
        m_maxPressure = 0.0f;
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
            QPointF trans = pos - m_endPos.pos();
            m_startPos.setPos(m_startPos.pos() + trans);
            m_endPos.setPos(m_endPos.pos() + trans);
        } else if (event->modifiers() == Qt::ShiftModifier) {
            m_endPos.setPos(straightLine(pos));
        } else {
            m_endPos.setPos(pos);
        }

        m_maxPressure = qMax(m_maxPressure, float(pressureToCurve(event->pressure())));
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

        if(m_bnVaryingEnds->isChecked()) {
            m_endPos = KisPaintInformation(
                m_endPos.pos(),
                PRESSURE_DEFAULT,
                m_cbTilt->isChecked() ? event->xTilt() : 0.0,
                m_cbTilt->isChecked() ? event->yTilt() : 0.0,
                nullKisVector2D(),
                m_cbRotation->isChecked() ? event->rotation() : 0.0,
                m_cbTangPressure->isChecked() ? event->tangentialPressure() : 0.0
            );
        }

        if (event->modifiers() == Qt::AltModifier) {
            QPointF trans = pos - m_endPos.pos();
            m_startPos.setPos(m_startPos.pos() + trans);
            m_endPos.setPos(m_endPos.pos() + trans);
        } else if (event->modifiers() == Qt::ShiftModifier) {
            m_endPos.setPos(straightLine(pos));
        } else {
            m_endPos.setPos(pos);
        }

        if (m_startPos.pos() == m_endPos.pos())
            return;

        if(m_cbPressure->isChecked()) {
            m_startPos.setPressure(m_maxPressure);
            m_endPos.setPressure(m_maxPressure);
        }

        NodePaintAbility nodeAbility = nodePaintAbility();
        if (nodeAbility == NONE) {
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

        if (nodeAbility == PAINT) {
            KisFigurePaintingToolHelper helper(i18nc("a straight drawn line", "Line"),
                                               image(),
                                               canvas()->resourceManager(),
                                               KisPainter::StrokeStyleBrush,
                                               KisPainter::FillStyleNone);
            helper.paintLine(m_startPos, m_endPos);
        }
        else {
            KoPathShape* path = new KoPathShape();
            path->setShapeId(KoPathShapeId);

            QTransform resolutionMatrix;
            resolutionMatrix.scale(1 / currentImage()->xRes(), 1 / currentImage()->yRes());
            path->moveTo(resolutionMatrix.map(m_startPos.pos()));
            path->lineTo(resolutionMatrix.map(m_endPos.pos()));
            path->normalize();

            KoShapeStroke* border = new KoShapeStroke(1.0, currentFgColor().toQColor());
            path->setStroke(border);

            KUndo2Command * cmd = canvas()->shapeController()->addShape(path);
            canvas()->addCommand(cmd);
        }
    }
    else {
        KisToolPaint::mouseReleaseEvent(event);
        return;
    }
    notifyModified();
}


QPointF KisToolLine::straightLine(QPointF point)
{
    const QPointF lineVector = point - m_startPos.pos();
    qreal lineAngle = std::atan2(lineVector.y(), lineVector.x());

    if (lineAngle < 0) {
        lineAngle += 2 * M_PI;
    }

    const qreal ANGLE_BETWEEN_CONSTRAINED_LINES = (2 * M_PI) / 24;

    const quint32 constrainedLineIndex = static_cast<quint32>((lineAngle / ANGLE_BETWEEN_CONSTRAINED_LINES) + 0.5);
    const qreal constrainedLineAngle = constrainedLineIndex * ANGLE_BETWEEN_CONSTRAINED_LINES;

    const qreal lineLength = std::sqrt((lineVector.x() * lineVector.x()) + (lineVector.y() * lineVector.y()));

    const QPointF constrainedLineVector(lineLength * std::cos(constrainedLineAngle), lineLength * std::sin(constrainedLineAngle));

    const QPointF result = m_startPos.pos() + constrainedLineVector;

    return result;
}


void KisToolLine::updatePreview()
{
    if (canvas()) {
        QRectF bound(m_startPos.pos(), m_endPos.pos());
        canvas()->updateCanvas(convertToPt(bound.normalized().adjusted(-3, -3, 3, 3)));
    }
}


void KisToolLine::paintLine(QPainter& gc, const QRect&)
{
    QPointF viewStartPos = pixelToView(m_startPos.pos());
    QPointF viewStartEnd = pixelToView(m_endPos.pos());

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

