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

#include <ksharedconfig.h>

#include <KoCanvasBase.h>
#include <KoPointerEvent.h>
#include <KoPathShape.h>
#include <KoShapeController.h>
#include <KoShapeStroke.h>

#include <kis_debug.h>
#include <kis_cursor.h>
#include <brushengine/kis_paintop_registry.h>
#include <kis_figure_painting_tool_helper.h>
#include <kis_canvas2.h>
#include <kis_action_registry.h>
#include <kis_painting_information_builder.h>

#include "kis_tool_line_helper.h"


const KisCoordinatesConverter* getCoordinatesConverter(KoCanvasBase * canvas)
{
    KisCanvas2 *kritaCanvas = dynamic_cast<KisCanvas2*>(canvas);
    return kritaCanvas->coordinatesConverter();
}


KisToolLine::KisToolLine(KoCanvasBase * canvas)
    : KisToolShape(canvas, KisCursor::load("tool_line_cursor.png", 6, 6)),
      m_showGuideline(true),
      m_strokeIsRunning(false),
      m_infoBuilder(new KisConverterPaintingInformationBuilder(getCoordinatesConverter(canvas))),
      m_helper(new KisToolLineHelper(m_infoBuilder.data(), kundo2_i18n("Draw Line"))),
      m_strokeUpdateCompressor(500, KisSignalCompressor::POSTPONE),
      m_longStrokeUpdateCompressor(1000, KisSignalCompressor::FIRST_INACTIVE)
{
    setObjectName("tool_line");

    setSupportOutline(true);

    connect(&m_strokeUpdateCompressor, SIGNAL(timeout()), SLOT(updateStroke()));
    connect(&m_longStrokeUpdateCompressor, SIGNAL(timeout()), SLOT(updateStroke()));
}

KisToolLine::~KisToolLine()
{
}

void KisToolLine::resetCursorStyle()
{
    KisToolPaint::resetCursorStyle();

    overrideCursorIfNotEditable();
}

void KisToolLine::activate(ToolActivation activation, const QSet<KoShape*> &shapes)
{
   KisToolPaint::activate(activation, shapes);
   configGroup =  KSharedConfig::openConfig()->group(toolId());
}

void KisToolLine::deactivate()
{
    KisToolPaint::deactivate();
    cancelStroke();
}

QWidget* KisToolLine::createOptionWidget()
{
    QWidget* widget = KisToolPaint::createOptionWidget();

    m_chkUseSensors = new QCheckBox(i18n("Use sensors"));
    addOptionWidgetOption(m_chkUseSensors);

    m_chkShowPreview = new QCheckBox(i18n("Show Preview"));
    addOptionWidgetOption(m_chkShowPreview);

    m_chkShowGuideline = new QCheckBox(i18n("Show Guideline"));
    addOptionWidgetOption(m_chkShowGuideline);

    // hook up connections for value changing
    connect(m_chkUseSensors, SIGNAL(clicked(bool)), this, SLOT(setUseSensors(bool)) );
    connect(m_chkShowPreview, SIGNAL(clicked(bool)), this, SLOT(setShowPreview(bool)) );
    connect(m_chkShowGuideline, SIGNAL(clicked(bool)), this, SLOT(setShowGuideline(bool)) );


    // read values in from configuration
    m_chkUseSensors->setChecked(configGroup.readEntry("useSensors", true));
    m_chkShowPreview->setChecked(configGroup.readEntry("showPreview", true));
    m_chkShowGuideline->setChecked(configGroup.readEntry("showGuideline", true));

    return widget;
}

void KisToolLine::setUseSensors(bool value)
{
    configGroup.writeEntry("useSensors", value);
}

void KisToolLine::setShowGuideline(bool value)
{
    m_showGuideline = value;
    configGroup.writeEntry("showGuideline", value);
}

void KisToolLine::setShowPreview(bool value)
{
    configGroup.writeEntry("showPreview", value);
}

void KisToolLine::requestStrokeCancellation()
{
    cancelStroke();
}

void KisToolLine::requestStrokeEnd()
{
    // Terminate any in-progress strokes
    if (nodePaintAbility() == PAINT && m_helper->isRunning()) {
        endStroke();
    }
}

void KisToolLine::updatePreviewTimer(bool showGuideline)
{
    // If the user disables the guideline, we will want to try to draw some
    // preview lines even if they're slow, so set the timer to FIRST_ACTIVE.
    if (showGuideline) {
        m_strokeUpdateCompressor.setMode(KisSignalCompressor::POSTPONE);
    } else {
        m_strokeUpdateCompressor.setMode(KisSignalCompressor::FIRST_ACTIVE);
    }
}


void KisToolLine::paint(QPainter& gc, const KoViewConverter &converter)
{
    Q_UNUSED(converter);

    if(mode() == KisTool::PAINT_MODE) {
        paintLine(gc,QRect());
    }
    KisToolPaint::paint(gc,converter);
}

void KisToolLine::beginPrimaryAction(KoPointerEvent *event)
{
    NodePaintAbility nodeAbility = nodePaintAbility();
    if (nodeAbility == UNPAINTABLE || !nodeEditable()) {
        event->ignore();
        return;
    }

    setMode(KisTool::PAINT_MODE);

    const KisToolShape::ShapeAddInfo info =
        shouldAddShape(currentNode());

    // Always show guideline on vector layers
    m_showGuideline = m_chkShowGuideline->isChecked() || nodeAbility != PAINT;
    updatePreviewTimer(m_showGuideline);
    m_helper->setEnabled((nodeAbility == PAINT && !info.shouldAddShape) || info.shouldAddSelectionShape);
    m_helper->setUseSensors(m_chkUseSensors->isChecked());
    m_helper->start(event, canvas()->resourceManager());

    m_startPoint = convertToPixelCoordAndSnap(event);
    m_endPoint = m_startPoint;
    m_lastUpdatedPoint = m_startPoint;

    m_strokeIsRunning = true;
}

void KisToolLine::updateStroke()
{
    if (!m_strokeIsRunning) return;

    m_helper->repaintLine(canvas()->resourceManager(),
                          image(),
                          currentNode(),
                          image().data());
}

void KisToolLine::continuePrimaryAction(KoPointerEvent *event)
{
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);
    if (!m_strokeIsRunning) return;

    // First ensure the old guideline is deleted
    updateGuideline();

    QPointF pos = convertToPixelCoordAndSnap(event);

    if (event->modifiers() == Qt::AltModifier) {
        QPointF trans = pos - m_endPoint;
        m_helper->translatePoints(trans);
        m_startPoint += trans;
        m_endPoint += trans;
    } else if (event->modifiers() == Qt::ShiftModifier) {
        pos = straightLine(pos);
        m_helper->addPoint(event, pos);
    } else {
        m_helper->addPoint(event, pos);
    }
    m_endPoint = pos;

    // Draw preview if requested
    if (m_chkShowPreview->isChecked()) {
        // If the cursor has moved a significant amount, immediately clear the
        // current preview and redraw. Otherwise, do slow redraws periodically.
        auto updateDistance = (pixelToView(m_lastUpdatedPoint) - pixelToView(pos)).manhattanLength();
        if (updateDistance > 10) {
            m_helper->clearPaint();
            m_longStrokeUpdateCompressor.stop();
            m_strokeUpdateCompressor.start();
            m_lastUpdatedPoint = pos;
        } else if (updateDistance > 1) {
            m_longStrokeUpdateCompressor.start();
        }
    }


    updateGuideline();
    KisToolPaint::requestUpdateOutline(event->point, event);
}

void KisToolLine::endPrimaryAction(KoPointerEvent *event)
{
    Q_UNUSED(event);
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);
    setMode(KisTool::HOVER_MODE);

    updateGuideline();
    endStroke();
}

void KisToolLine::endStroke()
{
    NodePaintAbility nodeAbility = nodePaintAbility();

    if (!m_strokeIsRunning || m_startPoint == m_endPoint || nodeAbility == UNPAINTABLE) {
        m_helper->clearPoints();
        return;
    }

    const KisToolShape::ShapeAddInfo info =
        shouldAddShape(currentNode());

    if ((nodeAbility == PAINT && !info.shouldAddShape) || info.shouldAddSelectionShape) {
        updateStroke();
        m_helper->end();
    }
    else {
        KoPathShape* path = new KoPathShape();
        path->setShapeId(KoPathShapeId);

        QTransform resolutionMatrix;
        resolutionMatrix.scale(1 / currentImage()->xRes(), 1 / currentImage()->yRes());
        path->moveTo(resolutionMatrix.map(m_startPoint));
        path->lineTo(resolutionMatrix.map(m_endPoint));
        path->normalize();

        KoShapeStrokeSP border(new KoShapeStroke(currentStrokeWidth(), currentFgColor().toQColor()));
        path->setStroke(border);

        KUndo2Command * cmd = canvas()->shapeController()->addShape(path, 0);
        canvas()->addCommand(cmd);
    }

    m_strokeIsRunning = false;
    m_endPoint = m_startPoint;
}

void KisToolLine::cancelStroke()
{
    if (!m_strokeIsRunning) return;
    if (m_startPoint == m_endPoint) return;

    /**
     * The actual stroke is run by the timer so it is a legal
     * situation when m_strokeIsRunning is true, but the actual redraw
     * stroke is not running.
     */
    if (m_helper->isRunning()) {
        m_helper->cancel();
    }


    m_strokeIsRunning = false;
    m_endPoint = m_startPoint;
}

QPointF KisToolLine::straightLine(QPointF point)
{
    const QPointF lineVector = point - m_startPoint;
    qreal lineAngle = std::atan2(lineVector.y(), lineVector.x());

    if (lineAngle < 0) {
        lineAngle += 2 * M_PI;
    }

    const qreal ANGLE_BETWEEN_CONSTRAINED_LINES = (2 * M_PI) / 24;

    const quint32 constrainedLineIndex = static_cast<quint32>((lineAngle / ANGLE_BETWEEN_CONSTRAINED_LINES) + 0.5);
    const qreal constrainedLineAngle = constrainedLineIndex * ANGLE_BETWEEN_CONSTRAINED_LINES;

    const qreal lineLength = std::sqrt((lineVector.x() * lineVector.x()) + (lineVector.y() * lineVector.y()));

    const QPointF constrainedLineVector(lineLength * std::cos(constrainedLineAngle), lineLength * std::sin(constrainedLineAngle));

    const QPointF result = m_startPoint + constrainedLineVector;

    return result;
}


void KisToolLine::updateGuideline()
{
    if (canvas()) {
        QRectF bound(m_startPoint, m_endPoint);
        canvas()->updateCanvas(convertToPt(bound.normalized().adjusted(-3, -3, 3, 3)));
    }
}


void KisToolLine::paintLine(QPainter& gc, const QRect&)
{
    QPointF viewStartPos = pixelToView(m_startPoint);
    QPointF viewStartEnd = pixelToView(m_endPoint);

    if (m_showGuideline && canvas()) {
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
