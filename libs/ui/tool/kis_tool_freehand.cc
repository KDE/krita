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
#include <QApplication>
#include <QDesktopWidget>

#include <Eigen/Core>

#include <kis_icon.h>
#include <KoPointerEvent.h>
#include <KoViewConverter.h>
#include <KoCanvasController.h>

//pop up palette
#include <kis_canvas_resource_provider.h>

// Krita/image
#include <kis_layer.h>
#include <kis_paint_layer.h>
#include <kis_painter.h>
#include <brushengine/kis_paintop.h>
#include <kis_selection.h>
#include <brushengine/kis_paintop_preset.h>


// Krita/ui
#include "kis_abstract_perspective_grid.h"
#include "kis_config.h"
#include "canvas/kis_canvas2.h"
#include "kis_cursor.h"
#include <KisViewManager.h>
#include <kis_painting_assistants_decoration.h>
#include "kis_painting_information_builder.h"
#include "kis_tool_freehand_helper.h"
#include "strokes/freehand_stroke.h"

using namespace std::placeholders; // For _1 placeholder


KisToolFreehand::KisToolFreehand(KoCanvasBase * canvas, const QCursor & cursor, const KUndo2MagicString &transactionText)
    : KisToolPaint(canvas, cursor),
      m_paintopBasedPickingInAction(false),
      m_brushResizeCompressor(200, std::bind(&KisToolFreehand::slotDoResizeBrush, this, _1))
{
    m_assistant = false;
    m_magnetism = 1.0;
    m_only_one_assistant = true;

    setSupportOutline(true);
    setMaskSyntheticEvents(KisConfig(true).disableTouchOnCanvas()); // Disallow mouse events from finger presses unless enabled

    m_infoBuilder = new KisToolFreehandPaintingInformationBuilder(this);
    m_helper = new KisToolFreehandHelper(m_infoBuilder, transactionText);

    connect(m_helper, SIGNAL(requestExplicitUpdateOutline()), SLOT(explicitUpdateOutline()));
}

KisToolFreehand::~KisToolFreehand()
{
    delete m_helper;
    delete m_infoBuilder;
}

void KisToolFreehand::mouseMoveEvent(KoPointerEvent *event)
{
    KisToolPaint::mouseMoveEvent(event);
    m_helper->cursorMoved(convertToPixelCoord(event));
}

KisSmoothingOptionsSP KisToolFreehand::smoothingOptions() const
{
    return m_helper->smoothingOptions();
}

void KisToolFreehand::resetCursorStyle()
{
    KisConfig cfg(true);

    switch (cfg.newCursorStyle()) {
    case CURSOR_STYLE_NO_CURSOR:
        useCursor(KisCursor::blankCursor());
        break;
    case CURSOR_STYLE_POINTER:
        useCursor(KisCursor::arrowCursor());
        break;
    case CURSOR_STYLE_SMALL_ROUND:
        useCursor(KisCursor::roundCursor());
        break;
    case CURSOR_STYLE_CROSSHAIR:
        useCursor(KisCursor::crossCursor());
        break;
    case CURSOR_STYLE_TRIANGLE_RIGHTHANDED:
        useCursor(KisCursor::triangleRightHandedCursor());
        break;
    case CURSOR_STYLE_TRIANGLE_LEFTHANDED:
        useCursor(KisCursor::triangleLeftHandedCursor());
        break;
    case CURSOR_STYLE_BLACK_PIXEL:
        useCursor(KisCursor::pixelBlackCursor());
        break;
    case CURSOR_STYLE_WHITE_PIXEL:
        useCursor(KisCursor::pixelWhiteCursor());
        break;
    case CURSOR_STYLE_TOOLICON:
    default:
        KisToolPaint::resetCursorStyle();
        break;
    }
}

KisPaintingInformationBuilder* KisToolFreehand::paintingInformationBuilder() const
{
    return m_infoBuilder;
}

void KisToolFreehand::resetHelper(KisToolFreehandHelper *helper)
{
    delete m_helper;
    m_helper = helper;
}

int KisToolFreehand::flags() const
{
    return KisTool::FLAG_USES_CUSTOM_COMPOSITEOP|KisTool::FLAG_USES_CUSTOM_PRESET
           |KisTool::FLAG_USES_CUSTOM_SIZE;
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
    m_helper->initPaint(event,
                        convertToPixelCoord(event),
                        canvas()->resourceManager(),
                        image(),
                        currentNode(),
                        image().data());
}

void KisToolFreehand::doStroke(KoPointerEvent *event)
{
    //set canvas information here?//
    KisCanvas2 *canvas2 = dynamic_cast<KisCanvas2 *>(canvas());
    if (canvas2) {
        m_helper->setCanvasHorizontalMirrorState(canvas2->xAxisMirrored());
        m_helper->setCanvasRotation(canvas2->rotationAngle());
    }
    m_helper->paintEvent(event);
}

void KisToolFreehand::endStroke()
{
    m_helper->endPaint();
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

    NodePaintAbility paintability = nodePaintAbility();
    // XXX: move this to KisTool and make it work properly for clone layers: for clone layers, the shape paint tools don't work either
    if (!nodeEditable() || paintability != PAINT) {
        if (paintability == KisToolPaint::VECTOR || paintability == KisToolPaint::CLONE){
            KisCanvas2 * kiscanvas = static_cast<KisCanvas2*>(canvas());
            QString message = i18n("The brush tool cannot paint on this layer.  Please select a paint layer or mask.");
            kiscanvas->viewManager()->showFloatingMessage(message, koIcon("object-locked"));
        }
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
    Q_FOREACH (const QPointer<KisAbstractPerspectiveGrid> grid, static_cast<KisCanvas2*>(canvas())->viewManager()->canvasResourceProvider()->perspectiveGrids()) {
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
                                            m_infoBuilder->pressureToCurve(event->pressure()),
                                            event->xTilt(), event->yTilt(),
                                            event->rotation(),
                                            event->tangentialPressure(),
                                            perspective, 0, 0),
                        event->modifiers(),
                        currentNode());
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
    if (tryPickByPaintOp(event, action)) {
        m_paintopBasedPickingInAction = true;
        return;
    }

    if (action != ChangeSize) {
        KisToolPaint::beginAlternateAction(event, action);
        return;
    }

    setMode(GESTURE_MODE);
    m_initialGestureDocPoint = event->point;
    m_initialGestureGlobalPoint = QCursor::pos();

    m_lastDocumentPoint = event->point;
    m_lastPaintOpSize = currentPaintOpPreset()->settings()->paintOpSize();
}

void KisToolFreehand::continueAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (tryPickByPaintOp(event, action) || m_paintopBasedPickingInAction) return;

    if (action != ChangeSize) {
        KisToolPaint::continueAlternateAction(event, action);
        return;
    }

    QPointF lastWidgetPosition = convertDocumentToWidget(m_lastDocumentPoint);
    QPointF actualWidgetPosition = convertDocumentToWidget(event->point);

    QPointF offset = actualWidgetPosition - lastWidgetPosition;

    KisCanvas2 *canvas2 = dynamic_cast<KisCanvas2 *>(canvas());
    QRect screenRect = QApplication::desktop()->screenGeometry();

    qreal scaleX = 0;
    qreal scaleY = 0;
    canvas2->coordinatesConverter()->imageScale(&scaleX, &scaleY);

    const qreal maxBrushSize = KisConfig(true).readEntry("maximumBrushSize", 1000);
    const qreal effectiveMaxDragSize = 0.5 * screenRect.width();
    const qreal effectiveMaxBrushSize = qMin(maxBrushSize, effectiveMaxDragSize / scaleX);

    const qreal scaleCoeff = effectiveMaxBrushSize / effectiveMaxDragSize;
    const qreal sizeDiff = scaleCoeff * offset.x() ;

    if (qAbs(sizeDiff) > 0.01) {
        KisPaintOpSettingsSP settings = currentPaintOpPreset()->settings();
        const qreal newSize = qBound(0.01, m_lastPaintOpSize + sizeDiff, maxBrushSize);

        settings->setPaintOpSize(newSize);
        requestUpdateOutline(m_initialGestureDocPoint, 0);
        //m_brushResizeCompressor.start(newSize);

        m_lastDocumentPoint = event->point;
        m_lastPaintOpSize = newSize;
    }
}

void KisToolFreehand::endAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (tryPickByPaintOp(event, action) || m_paintopBasedPickingInAction) {
        m_paintopBasedPickingInAction = false;
        return;
    }

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

void KisToolFreehand::setOnlyOneAssistantSnap(bool assistant)
{
    m_only_one_assistant = assistant;
}

void KisToolFreehand::slotDoResizeBrush(qreal newSize)
{
    KisPaintOpSettingsSP settings = currentPaintOpPreset()->settings();

    settings->setPaintOpSize(newSize);
    requestUpdateOutline(m_initialGestureDocPoint, 0);

}

QPointF KisToolFreehand::adjustPosition(const QPointF& point, const QPointF& strokeBegin)
{
    if (m_assistant && static_cast<KisCanvas2*>(canvas())->paintingAssistantsDecoration()) {
        static_cast<KisCanvas2*>(canvas())->paintingAssistantsDecoration()->setOnlyOneAssistantSnap(m_only_one_assistant);
        QPointF ap = static_cast<KisCanvas2*>(canvas())->paintingAssistantsDecoration()->adjustPosition(point, strokeBegin);
        return (1.0 - m_magnetism) * point + m_magnetism * ap;
    }
    return point;
}

qreal KisToolFreehand::calculatePerspective(const QPointF &documentPoint)
{
    qreal perspective = 1.0;
    Q_FOREACH (const QPointer<KisAbstractPerspectiveGrid> grid, static_cast<KisCanvas2*>(canvas())->viewManager()->canvasResourceProvider()->perspectiveGrids()) {
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
    QPointF imagePos = convertToPixelCoord(documentPos);

    if (currentPaintOpPreset())
        return m_helper->paintOpOutline(imagePos,
                                        event,
                                        currentPaintOpPreset()->settings(),
                                        outlineMode);
    else
        return QPainterPath();
}


