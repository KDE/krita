/*
 *  kis_tool_freehand.cc - part of Krita
 *
 *  Copyright (c) 2003-2007 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>
 *  Copyright (c) 2007,2008,2010 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_tool_freehand.h"

#include <QPainter>
#include <QRect>
#include <QThreadPool>

#include <KoIcon.h>
#include <KoPointerEvent.h>
#include <KoViewConverter.h>
#include <KoCanvasController.h>

//pop up palette
#include <kis_canvas_resource_provider.h>

// Krita/image
#include <kis_layer.h>
#include <kis_paint_layer.h>
#include <kis_painter.h>
#include <kis_paintop.h>
#include <kis_selection.h>
#include <kis_paintop_preset.h>


// Krita/ui
#include "kis_abstract_perspective_grid.h"
#include "kis_config.h"
#include "canvas/kis_canvas2.h"
#include "kis_cursor.h"
#include <KisViewManager.h>
#include <kis_painting_assistants_decoration.h>
#include "kis_painting_information_builder.h"
#include "kis_tool_freehand_helper.h"
#include "kis_recording_adapter.h"
#include "strokes/freehand_stroke.h"


KisToolFreehand::KisToolFreehand(KoCanvasBase * canvas, const QCursor & cursor, const KUndo2MagicString &transactionText)
    : KisToolPaint(canvas, cursor)
{
    m_assistant = false;
    m_magnetism = 1.0;

    setSupportOutline(true);

    m_infoBuilder = new KisToolFreehandPaintingInformationBuilder(this);
    m_recordingAdapter = new KisRecordingAdapter();
    m_helper = new KisToolFreehandHelper(m_infoBuilder, transactionText, m_recordingAdapter);

    connect(m_helper, SIGNAL(requestExplicitUpdateOutline()),
            SLOT(explicitUpdateOutline()));
}

KisToolFreehand::~KisToolFreehand()
{
    delete m_helper;
    delete m_recordingAdapter;
    delete m_infoBuilder;
}

KisSmoothingOptionsSP KisToolFreehand::smoothingOptions() const
{
    return m_helper->smoothingOptions();
}

void KisToolFreehand::resetCursorStyle()
{
    KisConfig cfg;

    switch (cfg.cursorStyle()) {
    case CURSOR_STYLE_CROSSHAIR:
    case CURSOR_STYLE_OUTLINE_CENTER_CROSS:
        useCursor(KisCursor::crossCursor());
        break;
    case CURSOR_STYLE_POINTER:
        useCursor(KisCursor::arrowCursor());
        break;
    case CURSOR_STYLE_NO_CURSOR:
        useCursor(KisCursor::blankCursor());
        break;
    case CURSOR_STYLE_SMALL_ROUND:
    case CURSOR_STYLE_OUTLINE_CENTER_DOT:
        useCursor(KisCursor::roundCursor());
        break;
    case CURSOR_STYLE_OUTLINE:
        useCursor(KisCursor::blankCursor());
        break;
    case CURSOR_STYLE_TRIANGLE_RIGHTHANDED:
    case CURSOR_STYLE_OUTLINE_TRIANGLE_RIGHTHANDED:
        useCursor(KisCursor::triangleRightHandedCursor());
        break;
    case CURSOR_STYLE_TRIANGLE_LEFTHANDED:
    case CURSOR_STYLE_OUTLINE_TRIANGLE_LEFTHANDED:
        useCursor(KisCursor::triangleLeftHandedCursor());
        break;
    case CURSOR_STYLE_TOOLICON:
    default:
        KisToolPaint::resetCursorStyle();
    }
}

KisPaintingInformationBuilder* KisToolFreehand::paintingInformationBuilder() const
{
    return m_infoBuilder;
}

KisRecordingAdapter* KisToolFreehand::recordingAdapter() const
{
    return m_recordingAdapter;
}

void KisToolFreehand::resetHelper(KisToolFreehandHelper *helper)
{
    delete m_helper;
    m_helper = helper;
}

int KisToolFreehand::flags() const
{
    return KisTool::FLAG_USES_CUSTOM_COMPOSITEOP|KisTool::FLAG_USES_CUSTOM_PRESET;
}

void KisToolFreehand::activate(ToolActivation activation, const QSet<KoShape*> &shapes)
{
    KisToolPaint::activate(activation, shapes);
}

void KisToolFreehand::deactivate()
{
    if (mode() == PAINT_MODE) {
        endStroke();
        setMode(KisTool::HOVER_MODE);
    }
    KisToolPaint::deactivate();
}

void KisToolFreehand::initStroke(KoPointerEvent *event)
{
    setCurrentNodeLocked(true);

    m_helper->initPaint(event, canvas()->resourceManager(),
                        image(),
                        currentNode(),
                        image().data(),
                        image()->postExecutionUndoAdapter());
}

void KisToolFreehand::doStroke(KoPointerEvent *event)
{
    m_helper->paint(event);
}

void KisToolFreehand::endStroke()
{
    m_helper->endPaint();
    setCurrentNodeLocked(false);
}

bool KisToolFreehand::primaryActionSupportsHiResEvents() const
{
    return true;
}

void KisToolFreehand::beginPrimaryAction(KoPointerEvent *event)
{
    // FIXME: workaround for the Duplicate Op
    tryPickByPaintOp(event, PickFgImage);

    requestUpdateOutline(event->point, event);

    if (!nodeEditable() || nodePaintAbility() != PAINT) {
        event->ignore();

        return;
    }

    setMode(KisTool::PAINT_MODE);

    KisCanvas2 *canvas2 = dynamic_cast<KisCanvas2 *>(canvas());
    if (canvas2) {
        canvas2->viewManager()->disableControls();
    }

    initStroke(event);
}

void KisToolFreehand::continuePrimaryAction(KoPointerEvent *event)
{
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);

    requestUpdateOutline(event->point, event);

    /**
     * Actual painting
     */
    doStroke(event);
}

void KisToolFreehand::endPrimaryAction(KoPointerEvent *event)
{
    Q_UNUSED(event);
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);

    endStroke();

    if (m_assistant && static_cast<KisCanvas2*>(canvas())->paintingAssistantsDecoration()) {
        static_cast<KisCanvas2*>(canvas())->paintingAssistantsDecoration()->endStroke();
    }

    notifyModified();
    KisCanvas2 *canvas2 = dynamic_cast<KisCanvas2 *>(canvas());
    if (canvas2) {
        canvas2->viewManager()->enableControls();
    }

    setMode(KisTool::HOVER_MODE);
}

bool KisToolFreehand::tryPickByPaintOp(KoPointerEvent *event, AlternateAction action)
{
    if (action != PickFgNode && action != PickFgImage) return false;

    /**
     * FIXME: we need some better way to implement modifiers
     * for a paintop level. This method is used in DuplicateOp only!
     */
    QPointF pos = adjustPosition(event->point, event->point);
    qreal perspective = 1.0;
    foreach (const QPointer<KisAbstractPerspectiveGrid> grid, static_cast<KisCanvas2*>(canvas())->viewManager()->resourceProvider()->perspectiveGrids()) {
        if (grid && grid->contains(pos)) {
            perspective = grid->distance(pos);
            break;
        }
    }
    if (!currentPaintOpPreset()) {
        return false;
    }
    bool paintOpIgnoredEvent = currentPaintOpPreset()->settings()->
        mousePressEvent(KisPaintInformation(convertToPixelCoord(event->point),
                                            pressureToCurve(event->pressure()),
                                            event->xTilt(), event->yTilt(),
                                            event->rotation(),
                                            event->tangentialPressure(),
                                            perspective, 0),
                        event->modifiers());
    return !paintOpIgnoredEvent;
}

void KisToolFreehand::activateAlternateAction(AlternateAction action)
{
    if (action != ChangeSize) {
        KisToolPaint::activateAlternateAction(action);
        return;
    }

    useCursor(KisCursor::blankCursor());
    setOutlineEnabled(true);
}

void KisToolFreehand::deactivateAlternateAction(AlternateAction action)
{
    if (action != ChangeSize) {
        KisToolPaint::deactivateAlternateAction(action);
        return;
    }

    resetCursorStyle();
    setOutlineEnabled(false);
}

void KisToolFreehand::beginAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (tryPickByPaintOp(event, action)) return;

    if (action != ChangeSize) {
        KisToolPaint::beginAlternateAction(event, action);
        return;
    }

    setMode(GESTURE_MODE);
    m_initialGestureDocPoint = event->point;
    m_initialGestureGlobalPoint = QCursor::pos();

    m_lastDocumentPoint = event->point;
}

void KisToolFreehand::continueAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (tryPickByPaintOp(event, action)) return;

    if (action != ChangeSize) {
        KisToolPaint::continueAlternateAction(event, action);
        return;
    }

    QPointF lastWidgetPosition = convertDocumentToWidget(m_lastDocumentPoint);
    QPointF actualWidgetPosition = convertDocumentToWidget(event->point);

    QPointF offset = actualWidgetPosition - lastWidgetPosition;

    /**
     * view pixels != widget pixels, but we do this anyway, we only
     * need to scale the gesture down, not rotate or anything
     */
    QPointF scaledOffset = canvas()->viewConverter()->viewToDocument(offset);

    if (qRound(scaledOffset.x()) != 0) {
        currentPaintOpPreset()->settings()->changePaintOpSize(scaledOffset.x(), scaledOffset.y());
        requestUpdateOutline(m_initialGestureDocPoint, 0);

        m_lastDocumentPoint = event->point;
    }
}

void KisToolFreehand::endAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (tryPickByPaintOp(event, action)) return;

    if (action != ChangeSize) {
        KisToolPaint::endAlternateAction(event, action);
        return;
    }

    QCursor::setPos(m_initialGestureGlobalPoint);
    requestUpdateOutline(m_initialGestureDocPoint, 0);

    setMode(HOVER_MODE);
}

bool KisToolFreehand::wantsAutoScroll() const
{
    return false;
}

void KisToolFreehand::setAssistant(bool assistant)
{
    m_assistant = assistant;
}

QPointF KisToolFreehand::adjustPosition(const QPointF& point, const QPointF& strokeBegin)
{
    if (m_assistant && static_cast<KisCanvas2*>(canvas())->paintingAssistantsDecoration()) {
        QPointF ap = static_cast<KisCanvas2*>(canvas())->paintingAssistantsDecoration()->adjustPosition(point, strokeBegin);
        return (1.0 - m_magnetism) * point + m_magnetism * ap;
    }
    return point;
}

qreal KisToolFreehand::calculatePerspective(const QPointF &documentPoint)
{
    qreal perspective = 1.0;
    foreach (const QPointer<KisAbstractPerspectiveGrid> grid, static_cast<KisCanvas2*>(canvas())->viewManager()->resourceProvider()->perspectiveGrids()) {
        if (grid && grid->contains(documentPoint)) {
            perspective = grid->distance(documentPoint);
            break;
        }
    }
    return perspective;
}

void KisToolFreehand::explicitUpdateOutline()
{
    requestUpdateOutline(m_outlineDocPoint, 0);
}

QPainterPath KisToolFreehand::getOutlinePath(const QPointF &documentPos,
                                             const KoPointerEvent *event,
                                             KisPaintOpSettings::OutlineMode outlineMode)
{
    QPointF imagePos = currentImage()->documentToPixel(documentPos);

    if (currentPaintOpPreset())
        return m_helper->paintOpOutline(imagePos,
                                        event,
                                        currentPaintOpPreset()->settings(),
                                        outlineMode);
    else
        return QPainterPath();
}


