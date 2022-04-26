/*
 *  SPDX-FileCopyrightText: 2022 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QScreen>

#include <KoPointerEvent.h>
#include <KoShapeController.h>
#include <KoViewConverter.h>
#include <KisViewManager.h>
#include <KoCanvasBase.h>
#include <kis_icon.h>
#include <kis_canvas2.h>
#include <kis_cubic_curve.h>
#include <kis_config.h>
#include <kis_config_notifier.h>
#include <kis_image_config.h>
#include <brushengine/kis_paintop_preset.h>

#include "KisToolBasicBrushBase.h"

KisToolBasicBrushBase::KisToolBasicBrushBase(KoCanvasBase * canvas, ToolType type, const QCursor & cursor)
    : KisToolShape(canvas, cursor)
    , m_type(type)
    , m_previewColor(0, 255, 0, 128)
{
    setSupportOutline(true);
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(updateSettings()));
    updateSettings();
}

KisToolBasicBrushBase::~KisToolBasicBrushBase()
{}

void KisToolBasicBrushBase::updateSettings()
{
    KisConfig cfg(true);
    // Pressure curve
    KisCubicCurve curve;
    curve.fromString(cfg.pressureTabletCurve());
    m_pressureSamples = curve.floatTransfer(levelOfPressureResolution + 1);
    // Outline options
    m_outlineStyle = cfg.newOutlineStyle();
    m_showOutlineWhilePainting = cfg.showOutlineWhilePainting();
    m_forceAlwaysFullSizedOutline = cfg.forceAlwaysFullSizedOutline();
}

void KisToolBasicBrushBase::mouseMoveEvent(KoPointerEvent *event)
{
    if (mode() == KisTool::HOVER_MODE) {
        m_lastPosition = convertToPixelCoord(event);
    }
    KisToolShape::mouseMoveEvent(event);
}

void KisToolBasicBrushBase::beginPrimaryAction(KoPointerEvent *event)
{
    NodePaintAbility paintability = nodePaintAbility();
    if ((m_type == PAINT && (!nodeEditable() || paintability == UNPAINTABLE || paintability  == KisToolPaint::CLONE || paintability == KisToolPaint::MYPAINTBRUSH_UNPAINTABLE)) || (m_type == SELECT && !selectionEditable())) {

        if (paintability == KisToolPaint::CLONE){
            KisCanvas2 * kiscanvas = static_cast<KisCanvas2*>(canvas());
            QString message = i18n("This tool cannot paint on clone layers.  Please select a paint or vector layer or mask.");
            kiscanvas->viewManager()->showFloatingMessage(message, koIcon("object-locked"));
        }

        if (paintability == KisToolPaint::MYPAINTBRUSH_UNPAINTABLE) {
            KisCanvas2 * kiscanvas = static_cast<KisCanvas2*>(canvas());
            QString message = i18n("The MyPaint Brush Engine is not available for this colorspace");
            kiscanvas->viewManager()->showFloatingMessage(message, koIcon("object-locked"));
        }

        event->ignore();
        return;
    }

    setMode(KisTool::PAINT_MODE);

    beginShape();

    const QPointF position = convertToPixelCoord(event);
    const qreal pressure = pressureToCurve(event->pressure());
    const qreal radius = pressure * currentPaintOpPreset()->settings()->paintOpSize() / 2.0;
    m_path = QPainterPath(position);
    m_path.setFillRule(Qt::WindingFill);
    m_path.addEllipse(position, radius, radius);

    m_lastPosition = position;
    m_lastPressure = pressure;

    update(m_path.boundingRect());
}

void KisToolBasicBrushBase::continuePrimaryAction(KoPointerEvent *event)
{
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);

    const QPointF position = convertToPixelCoord(event);
    const qreal pressure = pressureToCurve(event->pressure());
    const qreal brushRadius = currentPaintOpPreset()->settings()->paintOpSize() / 2.0;
    const QPainterPath segment = generateSegment(m_lastPosition, m_lastPressure * brushRadius, position, pressure * brushRadius);
    m_path.addPath(segment);

    m_lastPosition = position;
    m_lastPressure = pressure;

    requestUpdateOutline(event->point, event);
    update(segment.boundingRect());
}

void KisToolBasicBrushBase::endPrimaryAction(KoPointerEvent *event)
{
    Q_UNUSED(event);

    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);
    setMode(KisTool::HOVER_MODE);

    endShape();
    finishStroke(m_path);
}

void KisToolBasicBrushBase::activateAlternateAction(AlternateAction action)
{
    if (action != ChangeSize && action != ChangeSizeSnap) {
        KisToolShape::activateAlternateAction(action);
        return;
    }

    useCursor(KisCursor::blankCursor());
    setOutlineVisible(true);
}

void KisToolBasicBrushBase::deactivateAlternateAction(AlternateAction action)
{
    if (action != ChangeSize && action != ChangeSizeSnap) {
        KisToolShape::deactivateAlternateAction(action);
        return;
    }

    resetCursorStyle();
    setOutlineVisible(false);
}

void KisToolBasicBrushBase::beginAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (action != ChangeSize && action != ChangeSizeSnap) {
        KisToolShape::beginAlternateAction(event, action);
        return;
    }

    setMode(GESTURE_MODE);
    m_changeSizeInitialGestureDocPoint = event->point;
    m_changeSizeInitialGestureGlobalPoint = QCursor::pos();

    m_changeSizeLastDocumentPoint = event->point;
    m_changeSizeLastPaintOpSize = currentPaintOpPreset()->settings()->paintOpSize();
}

void KisToolBasicBrushBase::continueAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (action != ChangeSize && action != ChangeSizeSnap) {
        KisToolShape::continueAlternateAction(event, action);
        return;
    }

    QPointF lastWidgetPosition = convertDocumentToWidget(m_changeSizeLastDocumentPoint);
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

        qreal newSize = m_changeSizeLastPaintOpSize + sizeDiff;

        if (action == ChangeSizeSnap) {
            newSize = qMax(qRound(newSize), 1);
        }

        newSize = qBound(0.01, newSize, maxBrushSize);

        settings->setPaintOpSize(newSize);

        requestUpdateOutline(m_changeSizeInitialGestureDocPoint, 0);

        m_changeSizeLastDocumentPoint = event->point;
        m_changeSizeLastPaintOpSize = newSize;
    }
}

void KisToolBasicBrushBase::endAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (action != ChangeSize && action != ChangeSizeSnap) {
        KisToolShape::endAlternateAction(event, action);
        return;
    }

    QCursor::setPos(m_changeSizeInitialGestureGlobalPoint);
    requestUpdateOutline(m_changeSizeInitialGestureDocPoint, 0);

    setMode(HOVER_MODE);
}

void KisToolBasicBrushBase::update(const QRectF &strokeSegmentRect)
{
    QRectF segmentRect;
    QRectF outlineRect;
    // Segment rect
    if (mode() == KisTool::PAINT_MODE) {
        if (strokeSegmentRect.isValid()) {
            segmentRect = kisGrowRect(strokeSegmentRect, feedbackLineWidth);
        }
    }
    // Outline rect
    if (m_outlineStyle != OUTLINE_NONE &&
        (mode() != KisTool::PAINT_MODE || m_showOutlineWhilePainting)) {
        const qreal radius =
            m_forceAlwaysFullSizedOutline
            ? currentPaintOpPreset()->settings()->paintOpSize() / 2.0
            : m_lastPressure * currentPaintOpPreset()->settings()->paintOpSize() / 2.0;
        outlineRect = 
            kisGrowRect(
                QRectF(m_lastPosition - QPointF(radius, radius), m_lastPosition + QPointF(radius, radius)),
                feedbackLineWidth
            );
    }
    // Update
    if (segmentRect.isValid() && outlineRect.isValid()) {
        updateCanvasPixelRect(segmentRect.united(outlineRect));
    } else if (segmentRect.isValid()) {
        updateCanvasPixelRect(segmentRect);
    } else if (outlineRect.isValid()) {
        updateCanvasPixelRect(outlineRect);
    }
}

QPainterPath KisToolBasicBrushBase::getOutlinePath(const QPointF &documentPos,
                                                   const KoPointerEvent *event,
                                                   KisPaintOpSettings::OutlineMode outlineMode)
{
    Q_UNUSED(documentPos);
    Q_UNUSED(event);

    if (!outlineMode.isVisible) {
        return QPainterPath();
    }
    const qreal radius =
        mode() != KisTool::PAINT_MODE || outlineMode.forceFullSize
        ? currentPaintOpPreset()->settings()->paintOpSize() / 2.0
        : m_lastPressure * currentPaintOpPreset()->settings()->paintOpSize() / 2.0;
    QPainterPath outline;
    outline.addEllipse(m_lastPosition, radius, radius);
    return outline;
}

void KisToolBasicBrushBase::paint(QPainter &gc, const KoViewConverter &converter)
{
    if (mode() == KisTool::PAINT_MODE) {
        gc.fillPath(pixelToView(m_path), m_previewColor);
    }
    KisToolShape::paint(gc, converter);
}

void KisToolBasicBrushBase::activate(const QSet<KoShape*> &shapes)
{
    m_lastPressure = 1.0;
    
    KisToolShape::activate(shapes);
}

void KisToolBasicBrushBase::deactivate()
{
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    KIS_ASSERT_RECOVER_RETURN(kisCanvas);
    kisCanvas->updateCanvas();

    KisToolShape::deactivate();
}

void KisToolBasicBrushBase::setPreviewColor(const QColor &color)
{
    m_previewColor = color;
}

void KisToolBasicBrushBase::resetCursorStyle()
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

qreal KisToolBasicBrushBase::pressureToCurve(qreal pressure)
{
    return KisCubicCurve::interpolateLinear(pressure, m_pressureSamples);
}

QPainterPath KisToolBasicBrushBase::generateSegment(const QPointF &point1, qreal radius1, const QPointF &point2, qreal radius2) const
{
    const QPointF &p1 = radius1 < radius2 ? point2 : point1;
    const QPointF &p2 = radius1 < radius2 ? point1 : point2;
    const qreal &r1 = radius1 < radius2 ? radius2 : radius1;
    const qreal &r2 = radius1 < radius2 ? radius1 : radius2;
    const QPointF deltaP1P2 = p2 - p1;
    const qreal deltaR1R2 = r1 - r2;
    QPointF tangentPointP11, tangentPointP12, tangentPointP21, tangentPointP22;

    if (qFuzzyIsNull(deltaR1R2)) {
        // Same radius case
        const qreal deltaP1P2Length = std::sqrt(deltaP1P2.x() * deltaP1P2.x() + deltaP1P2.y() * deltaP1P2.y());
        const QPointF deltaP1P2Normalized = deltaP1P2 / deltaP1P2Length;
        tangentPointP11 = p1 + QPointF(deltaP1P2Normalized.y(), -deltaP1P2Normalized.x()) * r1;
        tangentPointP12 = p1 + QPointF(-deltaP1P2Normalized.y(), deltaP1P2Normalized.x()) * r1;
        tangentPointP21 = p2 + QPointF(deltaP1P2Normalized.y(), -deltaP1P2Normalized.x()) * r2;
        tangentPointP22 = p2 + QPointF(-deltaP1P2Normalized.y(), deltaP1P2Normalized.x()) * r2;
    } else {
        // General case
        const QPointF tangentIntersectionPoint(
            (p2.x() * r1 - p1.x() * r2) / deltaR1R2,
            (p2.y() * r1 - p1.y() * r2) / deltaR1R2
        );
        auto f = [](qreal t1, qreal t2, qreal t3, qreal t4, qreal sign) -> qreal
        {
            return (t1 + sign * t2) / t3 + t4;
        };
        {
            const qreal r1Squared = r1 * r1;
            const QPointF deltaP1TangentIntersectionPoint = tangentIntersectionPoint - p1;
            const qreal deltaP1TangentIntersectionPointLengthSquared = 
                deltaP1TangentIntersectionPoint.x() * deltaP1TangentIntersectionPoint.x() +
                deltaP1TangentIntersectionPoint.y() * deltaP1TangentIntersectionPoint.y();
            const QPointF t11 = r1Squared * deltaP1TangentIntersectionPoint;
            const QPointF t12 = r1 * deltaP1TangentIntersectionPoint * std::sqrt(deltaP1TangentIntersectionPointLengthSquared - r1Squared);
            tangentPointP11 = QPointF(
                f(t11.x(), t12.y(), deltaP1TangentIntersectionPointLengthSquared, p1.x(), 1.0),
                f(t11.y(), t12.x(), deltaP1TangentIntersectionPointLengthSquared, p1.y(), -1.0)
            );
            tangentPointP12 = QPointF(
                f(t11.x(), t12.y(), deltaP1TangentIntersectionPointLengthSquared, p1.x(), -1.0),
                f(t11.y(), t12.x(), deltaP1TangentIntersectionPointLengthSquared, p1.y(), 1.0)
            );
        }
        {
            const qreal r2Squared = r2 * r2;
            const QPointF deltaP2TangentIntersectionPoint = tangentIntersectionPoint - p2;
            const qreal deltaP2TangentIntersectionPointLengthSquared = 
                deltaP2TangentIntersectionPoint.x() * deltaP2TangentIntersectionPoint.x() +
                deltaP2TangentIntersectionPoint.y() * deltaP2TangentIntersectionPoint.y();
            const QPointF t11 = r2Squared * deltaP2TangentIntersectionPoint;
            const QPointF t12 = r2 * deltaP2TangentIntersectionPoint * std::sqrt(deltaP2TangentIntersectionPointLengthSquared - r2Squared);
            tangentPointP21 = QPointF(
                f(t11.x(), t12.y(), deltaP2TangentIntersectionPointLengthSquared, p2.x(), 1.0),
                f(t11.y(), t12.x(), deltaP2TangentIntersectionPointLengthSquared, p2.y(), -1.0)
            );
            tangentPointP22 = QPointF(
                f(t11.x(), t12.y(), deltaP2TangentIntersectionPointLengthSquared, p2.x(), -1.0),
                f(t11.y(), t12.x(), deltaP2TangentIntersectionPointLengthSquared, p2.y(), 1.0)
            );
        }
    }

    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.moveTo(tangentPointP11);
    path.lineTo(tangentPointP21);
    path.lineTo(tangentPointP22);
    path.lineTo(tangentPointP12);
    path.closeSubpath();
    path.addEllipse(point2, radius2, radius2);
    return path;
}
