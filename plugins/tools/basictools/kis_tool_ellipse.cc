/*
 *  kis_tool_ellipse.cc - part of Krayon
 *
 *  SPDX-FileCopyrightText: 2000 John Califf <jcaliff@compuzone.net>
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2004 Clarence Dang <dang@kde.org>
 *  SPDX-FileCopyrightText: 2009 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tool_ellipse.h"
#include <KoCanvasBase.h>
#include <KoPathShape.h>
#include <KoShapeStroke.h>

#include <QBrush>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLineF>
#include <QPen>
#include <QPixmap>

#include <KisOptionButtonStrip.h>
#include <KisViewManager.h>
#include <KoGroupButton.h>
#include <KoPointerEvent.h>
#include <canvas/kis_canvas2.h>
#include <kis_canvas_resource_provider.h>
#include <kis_icon.h>
#include <kis_shape_tool_helper.h>
#include "kis_figure_painting_tool_helper.h"
#include <brushengine/kis_paintop_preset.h>
#include <ksharedconfig.h>
#include <kis_rectangle_constraint_widget.h>

#include <cmath>

namespace
{
constexpr qreal MinAxisLength = 1.0;

QPointF rotate90(const QPointF &point)
{
    return QPointF(-point.y(), point.x());
}
}

KisToolEllipse::KisToolEllipse(KoCanvasBase * canvas)
        : KisToolEllipseBase(canvas, KisToolEllipseBase::PAINT, KisCursor::loadWithSize("tool_ellipse_cursor.svg", 32, 32, 6, 6))
{
    setObjectName("tool_ellipse");
    setSupportOutline(true);
    setIsOpacityPresetMode(true);

    KisCanvas2 *kritaCanvas = dynamic_cast<KisCanvas2*>(canvas);

    connect(kritaCanvas->viewManager()->canvasResourceProvider(), SIGNAL(sigEffectiveCompositeOpChanged()), SLOT(resetCursorStyle()));
}

KisToolEllipse::~KisToolEllipse()
{
}

QList<QPointer<QWidget>> KisToolEllipse::createOptionWidgets()
{
    QList<QPointer<QWidget>> widgets = KisToolEllipseBase::createOptionWidgets();

    QWidget *widget = new QWidget();
    widget->setObjectName("PlacementModeWidget");

    KisOptionButtonStrip *placementStrip = new KisOptionButtonStrip(widget);
    KoGroupButton *boundingBoxButton = placementStrip->addButton(i18nc("Ellipse tool placement mode", "Bounding Box"));
    KoGroupButton *threePointButton = placementStrip->addButton(i18nc("Ellipse tool placement mode", "3-Point"));
    boundingBoxButton->setToolTip(i18n("Place the ellipse inside a bounding box"));
    threePointButton->setToolTip(i18n("Place the ellipse by setting the major axis and width"));

    KConfigGroup cfg = KSharedConfig::openConfig()->group(toolId());
    m_placementMode = static_cast<PlacementMode>(cfg.readEntry("placementMode", int(BoundingBoxPlacement)));
    if (m_placementMode != ThreePointPlacement) {
        m_placementMode = BoundingBoxPlacement;
    }
    placementStrip->button(int(m_placementMode))->setChecked(true);

    connect(placementStrip, SIGNAL(buttonToggled(int,bool)), SLOT(slotPlacementModeChanged(int,bool)));

    /**
     * Disable geometry contraints widget in 3-point mode. It is used in
     * the bounding box mode only
     */
    Q_FOREACH (QPointer<QWidget> other, widgets) {
        if (qobject_cast<KisRectangleConstraintWidget*>(other)) {
            connect(placementStrip,
                    qOverload<int, bool>(&KisOptionButtonStrip::buttonToggled),
                    other,
                    [other](int mode, bool checked) {
                        if (!checked)
                            return;
                        other->setEnabled(static_cast<PlacementMode>(mode) == BoundingBoxPlacement);
                    });
            other->setEnabled(m_placementMode == BoundingBoxPlacement);
            break;
        }
    }

    QHBoxLayout *hLayout = new QHBoxLayout(widget);
    hLayout->addWidget(new QLabel(i18nc("Ellipse tool placement mode option label", "Placement:")), 0, Qt::AlignLeft);
    hLayout->addWidget(placementStrip, 1, Qt::AlignRight);
    widgets.prepend(widget);

    return widgets;
}

void KisToolEllipse::slotPlacementModeChanged(int mode, bool checked)
{
    if (!checked) {
        return;
    }

    cancelThreePointEllipse();
    setStatusText(QString());
    m_placementMode = mode == int(ThreePointPlacement) ? ThreePointPlacement : BoundingBoxPlacement;

    KConfigGroup cfg = KSharedConfig::openConfig()->group(toolId());
    cfg.writeEntry("placementMode", int(m_placementMode));
}

void KisToolEllipse::resetCursorStyle()
{
    if (isThreePointPlacement() && hasCompleteThreePointEllipse()) {
        useCursor(threePointConfirmCursor());
        overrideCursorIfNotEditable();
        return;
    }

    if (isEraser() && (nodePaintAbility() == NodePaintAbility::PAINT)) {
        useCursor(KisCursor::loadWithSize("tool_ellipse_eraser_cursor.svg", 32, 32, 6, 6));
    } else {
        KisToolEllipseBase::resetCursorStyle();
    }

    overrideCursorIfNotEditable();
}

bool KisToolEllipse::isThreePointPlacement() const
{
    return m_placementMode == ThreePointPlacement;
}

bool KisToolEllipse::canStartThreePointEllipse(KoPointerEvent *event)
{
    NodePaintAbility paintability = nodePaintAbility();
    if (!nodeEditable() || paintability == UNPAINTABLE || paintability == KisToolPaint::CLONE || paintability == KisToolPaint::MYPAINTBRUSH_UNPAINTABLE) {
        if (paintability == KisToolPaint::CLONE) {
            KisCanvas2 *kiscanvas = static_cast<KisCanvas2*>(canvas());
            kiscanvas->viewManager()->showFloatingMessage(
                i18n("This tool cannot paint on clone layers.  Please select a paint or vector layer or mask."),
                koIcon("object-locked"));
        }

        if (paintability == KisToolPaint::MYPAINTBRUSH_UNPAINTABLE) {
            KisCanvas2 *kiscanvas = static_cast<KisCanvas2*>(canvas());
            kiscanvas->viewManager()->showFloatingMessage(
                i18n("The MyPaint Brush Engine is not available for this colorspace"),
                koIcon("object-locked"));
        }

        event->ignore();
        return false;
    }

    return true;
}

bool KisToolEllipse::hasCompleteThreePointEllipse() const
{
    return m_threePointHandles.size() == 3;
}

QPointF KisToolEllipse::threePointPreviewPoint() const
{
    return hasCompleteThreePointEllipse() ? m_threePointHandles[2] : m_threePointHoverPoint;
}

int KisToolEllipse::threePointHandleAt(const QPointF &pos) const
{
    const QPointF viewPos = pixelToView(pos);
    const qreal radius = qMax<qreal>(grabSensitivity(), handleRadius() + 3);

    for (int i = m_threePointHandles.size() - 1; i >= 0; --i) {
        if (QLineF(viewPos, pixelToView(m_threePointHandles[i])).length() <= radius) {
            return i;
        }
    }

    return -1;
}

QCursor KisToolEllipse::threePointConfirmCursor() const
{
    static const QCursor cursor = [] {
        QPixmap pixmap(42, 32);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.drawPixmap(0, 0, QIcon(QStringLiteral(":/tool_ellipse_cursor.svg")).pixmap(32, 32));
        painter.setRenderHint(QPainter::Antialiasing, true);

        QPainterPath checkPath;
        checkPath.moveTo(31.0, 23.0);
        checkPath.lineTo(34.0, 26.0);
        checkPath.lineTo(41.0, 17.0);

        QPen outlinePen(Qt::white, 4.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter.setPen(outlinePen);
        painter.drawPath(checkPath);

        QPen checkPen(Qt::black, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter.setPen(checkPen);
        painter.drawPath(checkPath);
        painter.end();

        return QCursor(pixmap, 6, 6);
    }();

    return cursor;
}

void KisToolEllipse::updateThreePointCursor(const QPointF &pos)
{
    if (!isThreePointPlacement() || !hasCompleteThreePointEllipse()) {
        resetCursorStyle();
        return;
    }

    useCursor(threePointHandleAt(pos) >= 0 ? KisCursor::pointingHandCursor() : threePointConfirmCursor());
    overrideCursorIfNotEditable();
}

void KisToolEllipse::beginPrimaryAction(KoPointerEvent *event)
{
    if (!isThreePointPlacement()) {
        KisToolEllipseBase::beginPrimaryAction(event);
        return;
    }

    if (!canStartThreePointEllipse(event)) {
        return;
    }

    if (hasCompleteThreePointEllipse()) {
        m_threePointDraggedHandle = threePointHandleAt(convertToPixelCoordAndSnap(event, QPointF(), false));
        if (m_threePointDraggedHandle < 0) {
            m_threePointCommittedOnPress = true;
            commitThreePointEllipse();
            event->accept();
            return;
        } else {
            useCursor(KisCursor::pointingHandCursor());
        }
    }

    setMode(KisTool::PAINT_MODE);
    event->accept();
}

void KisToolEllipse::continuePrimaryAction(KoPointerEvent *event)
{
    if (!isThreePointPlacement()) {
        KisToolEllipseBase::continuePrimaryAction(event);
        return;
    }

    if (m_threePointCommittedOnPress) {
        event->accept();
        return;
    }

    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);
    const QPointF pos = convertToPixelCoordAndSnap(event, QPointF(), false);
    if (m_threePointDraggedHandle >= 0 && m_threePointDraggedHandle < m_threePointHandles.size()) {
        const QRectF oldRect = threePointUpdateRect();
        m_threePointHandles[m_threePointDraggedHandle] = pos;
        updateCanvasPixelRect(oldRect.united(threePointUpdateRect()));
    } else if (!hasCompleteThreePointEllipse()) {
        updateThreePointPreview(pos);
    }
    KisToolPaint::requestUpdateOutline(event->point, event);
}

void KisToolEllipse::endPrimaryAction(KoPointerEvent *event)
{
    if (!isThreePointPlacement()) {
        KisToolEllipseBase::endPrimaryAction(event);
        return;
    }

    if (m_threePointCommittedOnPress) {
        m_threePointCommittedOnPress = false;
        setMode(KisTool::HOVER_MODE);
        event->accept();
        return;
    }

    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);

    if (!event->spontaneous()) {
        cancelThreePointEllipse();
        event->accept();
        return;
    }

    const QPointF pos = convertToPixelCoordAndSnap(event, QPointF(), false);
    setMode(KisTool::HOVER_MODE);

    if (m_threePointDraggedHandle >= 0 && m_threePointDraggedHandle < m_threePointHandles.size()) {
        const QRectF oldRect = threePointUpdateRect();
        m_threePointHandles[m_threePointDraggedHandle] = pos;
        m_threePointDraggedHandle = -1;
        updateCanvasPixelRect(oldRect.united(threePointUpdateRect()));
        updateThreePointCursor(pos);
        event->accept();
        return;
    }

    if (!m_threePointEllipseActive) {
        beginShape();
        m_threePointEllipseActive = true;
    }

    if (!hasCompleteThreePointEllipse()) {
        if (!m_threePointHandles.isEmpty()) {
            updateCanvasPixelRect(threePointUpdateRect());
        }
        m_threePointHandles.append(pos);
        m_threePointHoverPoint = pos;
        updateCanvasPixelRect(threePointUpdateRect());

        if (m_threePointHandles.size() == 1) {
            setStatusText(i18n("Click to place the other end of the ellipse axis."));
        } else if (m_threePointHandles.size() == 2) {
            setStatusText(i18n("Click to set the ellipse width."));
        } else {
            setStatusText(i18n("Drag handles to adjust the ellipse. Press Enter to apply or Esc to cancel."));
            updateThreePointCursor(pos);
        }
    }

    event->accept();
}

void KisToolEllipse::mouseMoveEvent(KoPointerEvent *event)
{
    if (!isThreePointPlacement() || !m_threePointEllipseActive) {
        KisToolEllipseBase::mouseMoveEvent(event);
        return;
    }

    if (!hasCompleteThreePointEllipse()) {
        updateThreePointPreview(convertToPixelCoordAndSnap(event, QPointF(), false));
    } else {
        updateThreePointCursor(convertToPixelCoordAndSnap(event, QPointF(), false));
    }
    KisToolPaint::mouseMoveEvent(event);
}

void KisToolEllipse::paint(QPainter &gc, const KoViewConverter &converter)
{
    if (!isThreePointPlacement()) {
        KisToolEllipseBase::paint(gc, converter);
        return;
    }

    paintThreePointEllipse(gc);
    KisToolPaint::paint(gc, converter);
}

KisPopupWidgetInterface *KisToolEllipse::popupWidget()
{
    if (m_threePointEllipseActive) {
        return nullptr;
    }

    return KisToolEllipseBase::popupWidget();
}

void KisToolEllipse::requestStrokeEnd()
{
    if (!isThreePointPlacement()) {
        KisToolEllipseBase::requestStrokeEnd();
        return;
    }

    if (hasCompleteThreePointEllipse()) {
        commitThreePointEllipse();
    }
}

void KisToolEllipse::requestStrokeCancellation()
{
    if (!isThreePointPlacement()) {
        KisToolEllipseBase::requestStrokeCancellation();
        return;
    }

    cancelThreePointEllipse();
}

void KisToolEllipse::deactivate()
{
    if (isThreePointPlacement()) {
        cancelThreePointEllipse();
    }

    KisToolEllipseBase::deactivate();
}

void KisToolEllipse::updateThreePointPreview(const QPointF &pos)
{
    const QRectF oldRect = threePointUpdateRect();
    m_threePointHoverPoint = pos;
    updateCanvasPixelRect(oldRect.united(threePointUpdateRect()));
}

void KisToolEllipse::resetThreePointState()
{
    m_threePointHandles.clear();
    m_threePointDraggedHandle = -1;
    m_threePointEllipseActive = false;
    m_threePointCommittedOnPress = false;
    setMode(KisTool::HOVER_MODE);
    setStatusText(QString());
    resetCursorStyle();
}

void KisToolEllipse::cancelThreePointEllipse()
{
    if (!m_threePointEllipseActive && m_threePointHandles.isEmpty()) {
        return;
    }

    updateCanvasPixelRect(threePointUpdateRect());
    resetThreePointState();
}

void KisToolEllipse::commitThreePointEllipse()
{
    if (m_threePointCommitInProgress) {
        return;
    }

    QPainterPath path;
    if (!threePointEllipsePath(threePointPreviewPoint(), &path)) {
        return;
    }

    const QRectF updateRect = threePointUpdateRect().united(path.boundingRect());
    updateCanvasPixelRect(updateRect);

    // The painting helper starts a stroke, which re-emits requestStrokeEnd().
    // Clear the pending preview before painting to avoid recursive commits.
    m_threePointCommitInProgress = true;
    resetThreePointState();
    addThreePointEllipsePath(path);
    m_threePointCommitInProgress = false;

    updateCanvasPixelRect(updateRect);
}

void KisToolEllipse::paintThreePointEllipse(QPainter &gc)
{
    if (!m_threePointEllipseActive || m_threePointHandles.isEmpty()) {
        return;
    }

    QPainterPath path;
    const QPointF firstPoint = pixelToView(m_threePointHandles[0]);

    if (m_threePointHandles.size() == 1) {
        path.addPath(drawX(firstPoint));
        path.moveTo(firstPoint);
        path.lineTo(pixelToView(m_threePointHoverPoint));
    } else {
        QPainterPath ellipsePath;
        if (threePointEllipsePath(threePointPreviewPoint(), &ellipsePath)) {
            path.addPath(pixelToView(ellipsePath));
        } else {
            path.moveTo(firstPoint);
            path.lineTo(pixelToView(m_threePointHandles[1]));
        }
    }

    paintToolOutline(&gc, path);
    paintThreePointHandles(gc);
}

void KisToolEllipse::paintThreePointHandles(QPainter &gc)
{
    gc.save();
    gc.setRenderHint(QPainter::Antialiasing, true);
    gc.setPen(QPen(Qt::black, 1.0));
    gc.setBrush(QBrush(Qt::white));

    const qreal radius = qMax<qreal>(3.0, handleRadius() * 0.75);
    for (const QPointF &point : m_threePointHandles) {
        const QPointF viewPoint = pixelToView(point);
        gc.drawEllipse(QRectF(viewPoint - QPointF(radius, radius), QSizeF(radius * 2.0, radius * 2.0)));
    }

    gc.restore();
}

bool KisToolEllipse::threePointEllipsePath(const QPointF &thirdPoint, QPainterPath *path) const
{
    if (m_threePointHandles.size() < 2) {
        return false;
    }

    const QPointF p1 = m_threePointHandles[0];
    const QPointF p2 = m_threePointHandles[1];
    const QPointF center = (p1 + p2) * 0.5;
    const QPointF majorVector = p2 - p1;
    const qreal majorLength = std::hypot(majorVector.x(), majorVector.y());

    if (majorLength < MinAxisLength) {
        return false;
    }

    const qreal semiMajor = majorLength * 0.5;
    const QPointF majorUnit = majorVector / majorLength;
    const QPointF minorUnit = rotate90(majorUnit);
    const QPointF localPoint = thirdPoint - center;
    const qreal localX = QPointF::dotProduct(localPoint, majorUnit);
    const qreal localY = QPointF::dotProduct(localPoint, minorUnit);
    const qreal divisor = 1.0 - (localX * localX) / (semiMajor * semiMajor);

    if (divisor <= 0.0 || qFuzzyIsNull(localY)) {
        return false;
    }

    const qreal semiMinor = qAbs(localY) / std::sqrt(divisor);
    if (semiMinor <= 0.0) {
        return false;
    }

    QPainterPath canonicalPath;
    canonicalPath.addEllipse(QPointF(0, 0), semiMajor, semiMinor);

    QTransform ellipseTransform(majorUnit.x(), majorUnit.y(),
                                minorUnit.x(), minorUnit.y(),
                                center.x(), center.y());

    *path = ellipseTransform.map(canonicalPath);
    return true;
}

QRectF KisToolEllipse::threePointUpdateRect() const
{
    QRectF rect;

    for (const QPointF &point : m_threePointHandles) {
        rect |= QRectF(point, QSizeF(1.0, 1.0));
    }

    rect |= QRectF(m_threePointHoverPoint, QSizeF(1.0, 1.0));

    QPainterPath path;
    if (threePointEllipsePath(threePointPreviewPoint(), &path)) {
        rect |= path.boundingRect();
    }

    return rect.adjusted(-20.0, -20.0, 20.0, 20.0);
}

void KisToolEllipse::addThreePointEllipsePath(const QPainterPath &path)
{
    const KisToolShape::ShapeAddInfo info =
        shouldAddShape(currentNode());

    if (!info.shouldAddShape) {
        KisFigurePaintingToolHelper helper(kundo2_i18n("Draw Ellipse"),
                                           image(),
                                           currentNode(),
                                           canvas()->resourceManager(),
                                           strokeStyle(),
                                           fillStyle(),
                                           fillTransform());
        helper.paintPainterPath(path);
    } else {
        QPainterPath documentPath = path;
        if (image()) {
            QTransform pixelToDocument;
            pixelToDocument.scale(1.0 / image()->xRes(), 1.0 / image()->yRes());
            documentPath = pixelToDocument.map(path);
        }

        KoPathShape *shape = KoPathShape::createShapeFromPainterPath(documentPath);
        info.markAsSelectionShapeIfNeeded(shape);
        if (info.shouldAddSelectionShape) {
            shape->normalize();
            addShape(shape);
        } else {
            addPathShape(shape, kundo2_i18n("Draw Ellipse"));
        }
    }
}

void KisToolEllipse::finishRect(const QRectF& rect, qreal roundCornersX, qreal roundCornersY)
{
    Q_UNUSED(roundCornersX);
    Q_UNUSED(roundCornersY);

    if (rect.isEmpty())
        return;

    const KisToolShape::ShapeAddInfo info =
        shouldAddShape(currentNode());

    if (!info.shouldAddShape) {
        KisFigurePaintingToolHelper helper(kundo2_i18n("Draw Ellipse"),
                                           image(),
                                           currentNode(),
                                           canvas()->resourceManager(),
                                           strokeStyle(),
                                           fillStyle(),
                                           fillTransform());
        QPainterPath path;
        path.addEllipse(rect);
        getRotatedPath(path, rect.center(), getRotationAngle());
        helper.paintPainterPath(path);
    } else {
        KisResourcesSnapshot resources(image(),
                                       currentNode(),
                                       canvas()->resourceManager());
        QRectF r = convertToPt(rect);
        KoShape* shape = KisShapeToolHelper::createEllipseShape(r);
        shape->rotate(qRadiansToDegrees(getRotationAngle()));
        KoShapeStrokeSP border(new KoShapeStroke(currentStrokeWidth(), resources.currentFgColor().toQColor()));
        shape->setStroke(border);

        info.markAsSelectionShapeIfNeeded(shape);

        addShape(shape);
    }
}

bool KisToolEllipse::supportsPaintingAssistants() const
{
    return true;
}
