/*
 *  kis_tool_freehand.cc - part of Krita
 *
 *  SPDX-FileCopyrightText: 2003-2007 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2004 Bart Coppens <kde@bartcoppens.be>
 *  SPDX-FileCopyrightText: 2007, 2008, 2010 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2009 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tool_freehand.h"
#include <QPainter>
#include <QRect>
#include <QThreadPool>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>

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
#include "kis_image_config.h"
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
      m_paintopBasedSamplingInAction(false),
      m_brushResizeCompressor(200, std::bind(&KisToolFreehand::slotDoResizeBrush, this, _1))
{
    m_assistant = false;
    m_magnetism = 1.0;
    m_only_one_assistant = true;
    m_eraser_snapping = false;

    setSupportOutline(true);
    setMaskSyntheticEvents(KisConfig(true).disableTouchOnCanvas()); // Disallow mouse events from finger presses unless enabled

    m_infoBuilder = new KisToolFreehandPaintingInformationBuilder(this);
    m_helper = new KisToolFreehandHelper(m_infoBuilder, canvas->resourceManager(), transactionText);

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

void KisToolFreehand::activate(const QSet<KoShape*> &shapes)
{
    KisToolPaint::activate(shapes);
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
                        image(),
                        currentNode(),
                        image().data());
}

void KisToolFreehand::doStroke(KoPointerEvent *event)
{
    m_helper->paintEvent(event);
}

void KisToolFreehand::endStroke()
{
    m_helper->endPaint();
    bool paintOpIgnoredEvent = currentPaintOpPreset()->settings()->mouseReleaseEvent();
    Q_UNUSED(paintOpIgnoredEvent);
}

bool KisToolFreehand::primaryActionSupportsHiResEvents() const
{
    return true;
}

void KisToolFreehand::beginPrimaryAction(KoPointerEvent *event)
{
    // FIXME: workaround for the Duplicate Op
    trySampleByPaintOp(event, SampleFgImage);

    requestUpdateOutline(event->point, event);

    NodePaintAbility paintability = nodePaintAbility();
    // XXX: move this to KisTool and make it work properly for clone layers: for clone layers, the shape paint tools don't work either
    if (!nodeEditable() || paintability != PAINT) {
        if (paintability == KisToolPaint::VECTOR || paintability == KisToolPaint::CLONE){
            KisCanvas2 * kiscanvas = static_cast<KisCanvas2*>(canvas());
            QString message = i18n("The brush tool cannot paint on this layer.  Please select a paint layer or mask.");
            kiscanvas->viewManager()->showFloatingMessage(message, koIcon("object-locked"));
        }
        else if (paintability == MYPAINTBRUSH_UNPAINTABLE) {
            KisCanvas2 * kiscanvas = static_cast<KisCanvas2*>(canvas());
            QString message = i18n("The MyPaint Brush Engine is not available for this colorspace");
            kiscanvas->viewManager()->showFloatingMessage(message, koIcon("object-locked"));
        }
        event->ignore();

        return;
    }

    KIS_SAFE_ASSERT_RECOVER_RETURN(!m_helper->isRunning());

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

    KisCanvas2 *canvas2 = dynamic_cast<KisCanvas2 *>(canvas());
    if (canvas2) {
        canvas2->viewManager()->enableControls();
    }

    setMode(KisTool::HOVER_MODE);
}

bool KisToolFreehand::trySampleByPaintOp(KoPointerEvent *event, AlternateAction action)
{
    if (action != SampleFgNode && action != SampleFgImage) return false;

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
    KisPaintInformation info(convertToPixelCoord(event->point),
                             m_infoBuilder->pressureToCurve(event->pressure()),
                             event->xTilt(), event->yTilt(),
                             event->rotation(),
                             event->tangentialPressure(),
                             perspective, 0, 0);
    info.setRandomSource(new KisRandomSource());
    info.setPerStrokeRandomSource(new KisPerStrokeRandomSource());

    bool paintOpIgnoredEvent = currentPaintOpPreset()->settings()->mousePressEvent(info,
                                                                                   event->modifiers(),
                                                                                   currentNode());
    // DuplicateOP during the sampling of new source point (origin)
    // is the only paintop that returns "false" here
    return !paintOpIgnoredEvent;
}

void KisToolFreehand::activateAlternateAction(AlternateAction action)
{
    if (action != ChangeSize && action != ChangeSizeSnap) {
        KisToolPaint::activateAlternateAction(action);
        return;
    }

    useCursor(KisCursor::blankCursor());
    setOutlineEnabled(true);
}

void KisToolFreehand::deactivateAlternateAction(AlternateAction action)
{
    if (action != ChangeSize && action != ChangeSizeSnap) {
        KisToolPaint::deactivateAlternateAction(action);
        return;
    }

    resetCursorStyle();
    setOutlineEnabled(false);
}

void KisToolFreehand::beginAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (trySampleByPaintOp(event, action)) {
        m_paintopBasedSamplingInAction = true;
        return;
    }

    if (action != ChangeSize && action != ChangeSizeSnap) {
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
    if (trySampleByPaintOp(event, action) || m_paintopBasedSamplingInAction) return;

    if (action != ChangeSize && action != ChangeSizeSnap) {
        KisToolPaint::continueAlternateAction(event, action);
        return;
    }

    QPointF lastWidgetPosition = convertDocumentToWidget(m_lastDocumentPoint);
    QPointF actualWidgetPosition = convertDocumentToWidget(event->point);

    QPointF offset = actualWidgetPosition - lastWidgetPosition;

    KisCanvas2 *canvas2 = dynamic_cast<KisCanvas2 *>(canvas());
    QRect screenRect = QGuiApplication::primaryScreen()->availableVirtualGeometry();

    qreal scaleX = 0;
    qreal scaleY = 0;
    canvas2->coordinatesConverter()->imageScale(&scaleX, &scaleY);

    const qreal maxBrushSize = KisImageConfig(true).maxBrushSize();
    const qreal effectiveMaxDragSize = 0.5 * screenRect.width();
    const qreal effectiveMaxBrushSize = qMin(maxBrushSize, effectiveMaxDragSize / scaleX);

    const qreal scaleCoeff = effectiveMaxBrushSize / effectiveMaxDragSize;
    const qreal sizeDiff = scaleCoeff * offset.x() ;

    if (qAbs(sizeDiff) > 0.01) {
        KisPaintOpSettingsSP settings = currentPaintOpPreset()->settings();

        qreal newSize = m_lastPaintOpSize + sizeDiff;

        if (action == ChangeSizeSnap) {
            newSize = qMax(qRound(newSize), 1);
        }

        newSize = qBound(0.01, newSize, maxBrushSize);

        settings->setPaintOpSize(newSize);

        requestUpdateOutline(m_initialGestureDocPoint, 0);
        //m_brushResizeCompressor.start(newSize);

        m_lastDocumentPoint = event->point;
        m_lastPaintOpSize = newSize;
    }
}

void KisToolFreehand::endAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (trySampleByPaintOp(event, action) || m_paintopBasedSamplingInAction) {
        m_paintopBasedSamplingInAction = false;
        return;
    }

    if (action != ChangeSize && action != ChangeSizeSnap) {
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

void KisToolFreehand::setSnapEraser(bool assistant)
{
    m_eraser_snapping = assistant;
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
        KisCanvas2* c = static_cast<KisCanvas2*>(canvas());
        c->paintingAssistantsDecoration()->setOnlyOneAssistantSnap(m_only_one_assistant);
        c->paintingAssistantsDecoration()->setEraserSnap(m_eraser_snapping);
        QPointF ap = c->paintingAssistantsDecoration()->adjustPosition(point, strokeBegin);
        QPointF fp = (1.0 - m_magnetism) * point + m_magnetism * ap;
        // Report the final position back to the assistant so the guides
        // can follow the brush
        c->paintingAssistantsDecoration()->setAdjustedBrushPosition(fp);
        return fp;
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
    if (currentPaintOpPreset())
        return m_helper->paintOpOutline(convertToPixelCoord(documentPos),
                                        event,
                                        currentPaintOpPreset()->settings(),
                                        outlineMode);
    else
        return QPainterPath();
}


