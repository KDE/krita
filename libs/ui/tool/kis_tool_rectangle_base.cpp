/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_tool_rectangle_base.h"

#include <QtCore/qmath.h>

#include "KisViewManager.h"
#include "kis_canvas2.h"
#include <KisOptionCollectionWidget.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoPointerEvent.h>
#include <KoViewConverter.h>
#include <input/kis_extended_modifiers_mapper.h>
#include <kis_icon.h>

#include "kis_rectangle_constraint_widget.h"

KisToolRectangleBase::KisToolRectangleBase(KoCanvasBase * canvas, KisToolRectangleBase::ToolType type, const QCursor & cursor)
    : KisToolShape(canvas, cursor)
    , m_dragStart(0, 0)
    , m_dragEnd(0, 0)
    , m_type(type)
    , m_isRatioForced(false)
    , m_isWidthForced(false)
    , m_isHeightForced(false)
    , m_rotateActive(false)
    , m_forcedRatio(1.0)
    , m_forcedWidth(0)
    , m_forcedHeight(0)
    , m_roundCornersX(0)
    , m_roundCornersY(0)
    , m_referenceAngle(0)
    , m_angle(0)
    , m_angleBuffer(0)
    , m_currentModifiers(Qt::NoModifier)
{
}


QList<QPointer<QWidget> > KisToolRectangleBase::createOptionWidgets()
{
    QList<QPointer<QWidget>> widgetsList = KisToolShape::createOptionWidgets();

    KisRectangleConstraintWidget *widget =
        new KisRectangleConstraintWidget(0, this, showRoundCornersGUI());

    if (widgetsList.size() > 0
        && dynamic_cast<KisOptionCollectionWidget *>(
            widgetsList.first().data())) {
        KisOptionCollectionWidget *baseOptions =
            dynamic_cast<KisOptionCollectionWidget *>(
                widgetsList.first().data());
        KisOptionCollectionWidgetWithHeader *sectionRectangle =
            new KisOptionCollectionWidgetWithHeader(widget->windowTitle());
        sectionRectangle->appendWidget("rectangleConstraintWidget", widget);
        baseOptions->appendWidget("sectionRectangle", sectionRectangle);
    } else {
        widget->setContentsMargins(10, 10, 10, 10);
        widgetsList.append(widget);
    }

    return widgetsList;
}

void KisToolRectangleBase::constraintsChanged(bool forceRatio, bool forceWidth, bool forceHeight, float ratio, float width, float height)
{
    m_isWidthForced = forceWidth;
    m_isHeightForced = forceHeight;
    m_isRatioForced = forceRatio;

    m_forcedHeight = height;
    m_forcedWidth = width;
    m_forcedRatio = ratio;

    // Avoid division by zero in size calculations
    if (ratio < 0.0001f)
        m_isRatioForced = false;
}

void KisToolRectangleBase::roundCornersChanged(int rx, int ry)
{
    m_roundCornersX = rx;
    m_roundCornersY = ry;
}

void KisToolRectangleBase::showSize()
{
    KisCanvas2 *kisCanvas =dynamic_cast<KisCanvas2*>(canvas());
    KIS_SAFE_ASSERT_RECOVER_RETURN(kisCanvas);
    kisCanvas->viewManager()->showFloatingMessage(i18n("Width: %1 px\nHeight: %2 px"
                                                       , createRect(m_dragStart, m_dragEnd).width()
                                                       , createRect(m_dragStart, m_dragEnd).height()), QIcon(), 1000
                                                       , KisFloatingMessage::High,  Qt::AlignLeft | Qt::TextWordWrap | Qt::AlignVCenter);

}
void KisToolRectangleBase::paint(QPainter& gc, const KoViewConverter &converter)
{
    if(mode() == KisTool::PAINT_MODE) {
        paintRectangle(gc, createRect(m_dragStart, m_dragEnd));
    }

    KisToolPaint::paint(gc, converter);
}

void KisToolRectangleBase::activate(const QSet<KoShape *> &shapes)
{
    KisToolShape::activate(shapes);

    emit sigRequestReloadConfig();
}

void KisToolRectangleBase::deactivate()
{
    cancelStroke();
    KisToolShape::deactivate();
}

void KisToolRectangleBase::keyPressEvent(QKeyEvent *event) {
    const Qt::Key key = KisExtendedModifiersMapper::workaroundShiftAltMetaHell(event);

    if (key == Qt::Key_Control) {
        m_currentModifiers |= Qt::ControlModifier;
    } else if (key == Qt::Key_Shift) {
        m_currentModifiers |= Qt::ShiftModifier;
    } else if (key == Qt::Key_Alt) {
        m_currentModifiers |= Qt::AltModifier;
    }

    KisToolShape::keyPressEvent(event);
}

void KisToolRectangleBase::keyReleaseEvent(QKeyEvent *event) {
    const Qt::Key key = KisExtendedModifiersMapper::workaroundShiftAltMetaHell(event);

    if (key == Qt::Key_Control) {
        m_currentModifiers &= ~Qt::ControlModifier;
    } else if (key == Qt::Key_Shift) {
        m_currentModifiers &= ~Qt::ShiftModifier;
    } else if (key == Qt::Key_Alt) {
        m_currentModifiers &= ~Qt::AltModifier;
    }

    KisToolShape::keyReleaseEvent(event);
}

void KisToolRectangleBase::beginPrimaryAction(KoPointerEvent *event)
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

    m_currentModifiers = Qt::NoModifier;

    QPointF pos = convertToPixelCoordAndSnap(event, QPointF(), false);
    m_dragStart = m_dragCenter = pos;
    m_angle = m_angleBuffer = 0;
    m_rotateActive = false;

    QSizeF area = QSizeF(0,0);

    applyConstraints(area, false);

    m_dragEnd.setX(m_dragStart.x() + area.width());
    m_dragEnd.setY(m_dragStart.y() + area.height());

    m_dragCenter = QPointF((m_dragStart.x() + m_dragEnd.x()) / 2,
                           (m_dragStart.y() + m_dragEnd.y()) / 2);
    showSize();
    event->accept();
}

bool KisToolRectangleBase::isFixedSize() {
  if (m_isWidthForced && m_isHeightForced) return true;
  if (m_isRatioForced && (m_isWidthForced || m_isHeightForced)) return true;

  return false;
}

void KisToolRectangleBase::applyConstraints(QSizeF &area, bool overrideRatio) {
  if (m_isWidthForced) {
    area.setWidth(m_forcedWidth);
  }
  if (m_isHeightForced) {
    area.setHeight(m_forcedHeight);
  }

  if (m_isHeightForced && m_isWidthForced) return;

  if (m_isRatioForced || overrideRatio) {
    float ratio = m_isRatioForced ? m_forcedRatio : 1.0f;

    if (m_isWidthForced) {
      area.setHeight(area.width() / ratio);
    } else {
      area.setWidth(area.height() * ratio);
    }
  }
}

void KisToolRectangleBase::continuePrimaryAction(KoPointerEvent *event)
{
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);

    bool constraintToggle = m_currentModifiers & Qt::ShiftModifier;
    bool translateMode = m_currentModifiers & Qt::AltModifier;
    bool expandFromCenter = m_currentModifiers & Qt::ControlModifier;

    bool rotateMode = expandFromCenter && translateMode;
    bool fixedSize = isFixedSize() && !constraintToggle;

    QPointF pos = convertToPixelCoordAndSnap(event, QPointF(), false);

    if (rotateMode) {
        QPointF angleVector;
        if (!m_rotateActive) {
            m_rotateActive = true;
            angleVector = (fixedSize)? m_dragEnd: pos;
            angleVector -= m_dragStart;
            m_referenceAngle = atan2(angleVector.y(), angleVector.x());
        }
        angleVector = pos - m_dragStart;
        qreal a2 = atan2(angleVector.y(), angleVector.x());
        m_angleBuffer = a2 - m_referenceAngle;
    } else {
        m_rotateActive = false;
        m_angle += m_angleBuffer;
        m_angleBuffer = 0;
    }

    if (fixedSize && !rotateMode) {
      m_dragStart = pos;
    } else if (translateMode && !rotateMode) {
      QPointF trans = pos - m_dragEnd;
      m_dragStart += trans;
      m_dragEnd += trans;

    }

    QPointF diag = pos - m_dragStart;
    QTransform t1, t2;
    t1.rotateRadians(-getRotationAngle());
    QPointF baseDiag = t1.map(diag);
    QSizeF area = QSizeF(fabs(baseDiag.x()), fabs(baseDiag.y()));

    bool overrideRatio = constraintToggle && !(m_isHeightForced || m_isWidthForced || m_isRatioForced);
    if (!constraintToggle || overrideRatio) {
      applyConstraints(area, overrideRatio);
    }

    baseDiag = QPointF(
      (baseDiag.x() < 0) ? -area.width() : area.width(),
      (baseDiag.y() < 0) ? -area.height() : area.height()
    );

    t2.rotateRadians(getRotationAngle());
    diag = t2.map(baseDiag);

    // resize around center point?
    if (expandFromCenter && !fixedSize && !rotateMode) {
      m_dragStart = m_dragCenter - diag / 2;
      m_dragEnd = m_dragCenter + diag / 2;
    } else {
      m_dragEnd = m_dragStart + diag;
    }

    if(!translateMode) {
        showSize();
    }
    else {
        KisCanvas2 *kisCanvas =dynamic_cast<KisCanvas2*>(canvas());
        KIS_ASSERT(kisCanvas);
        kisCanvas->viewManager()->showFloatingMessage(i18n("X: %1 px\nY: %2 px"
                                                           , QString::number(m_dragStart.x(), 'f', 1)
                                                           , QString::number(m_dragStart.y(), 'f', 1)), QIcon(), 1000
                                                           , KisFloatingMessage::High,  Qt::AlignLeft | Qt::TextWordWrap | Qt::AlignVCenter);
    }
    updateArea();
    m_dragCenter = QPointF((m_dragStart.x() + m_dragEnd.x()) / 2,
                           (m_dragStart.y() + m_dragEnd.y()) / 2);

    KisToolPaint::requestUpdateOutline(event->point, event);
}

void KisToolRectangleBase::endPrimaryAction(KoPointerEvent *event)
{
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);
    // If the event was not originated by the user releasing the button
    // (for example due to the canvas loosing focus), then we just cancel the
    // operation. This prevents some issues with shapes being added after
    // the image was closed while the shape was being made
    if (event->spontaneous()) {
        endStroke();
    } else {
        cancelStroke();
    }
    event->accept();
}

void KisToolRectangleBase::requestStrokeEnd()
{
    if (mode() != KisTool::PAINT_MODE) {
        return;
    }
    endStroke();
}

void KisToolRectangleBase::requestStrokeCancellation()
{
    if (mode() != KisTool::PAINT_MODE) {
        return;
    }
    cancelStroke();
}

void KisToolRectangleBase::endStroke()
{
    setMode(KisTool::HOVER_MODE);
    updateArea();
    finishRect(createRect(m_dragStart, m_dragEnd), m_roundCornersX, m_roundCornersY);
    endShape();
}

void KisToolRectangleBase::cancelStroke()
{
    setMode(KisTool::HOVER_MODE);
    updateArea();
    endShape();
}

QRectF KisToolRectangleBase::createRect(const QPointF &start, const QPointF &end)
{
    QTransform t;
    t.translate(start.x(), start.y());
    t.rotateRadians(-getRotationAngle());
    t.translate(-start.x(), -start.y());
    const QTransform tInv = t.inverted();

    const QPointF end1 = t.map(end);
    const QPointF newStart(qRound(start.x()), qRound(start.y()));
    const QPointF newEnd(qRound(end1.x()), qRound(end1.y()));
    const QPointF newCenter = (newStart + newEnd) / 2.0;
   
    QRectF result(newStart, newEnd);
    result.moveCenter(tInv.map(newCenter));

    return result.normalized();
}

bool KisToolRectangleBase::showRoundCornersGUI() const
{
    return true;
}

void KisToolRectangleBase::paintRectangle(QPainter &gc, const QRectF &imageRect)
{
    KIS_ASSERT_RECOVER_RETURN(canvas());

    const QRect viewRect = pixelToView(imageRect).toAlignedRect();

    KisCanvas2 *kritaCanvas = dynamic_cast<KisCanvas2*>(canvas());
    KIS_SAFE_ASSERT_RECOVER_RETURN(kritaCanvas);

    const KisCoordinatesConverter *converter = kritaCanvas->coordinatesConverter();
    const qreal roundCornersX = converter->effectiveZoom() * m_roundCornersX;
    const qreal roundCornersY = converter->effectiveZoom() * m_roundCornersY;

    QPainterPath path;
    if (m_roundCornersX > 0 || m_roundCornersY > 0) {
        path.addRoundedRect(viewRect,
                            roundCornersX, roundCornersY);
    } else {
        path.addRect(viewRect);
    }

    getRotatedPath(path, viewRect.center(), getRotationAngle());
    path.addPath(drawX(pixelToView(m_dragStart)));
    path.addPath(drawX(pixelToView(m_dragCenter)));
    paintToolOutline(&gc, path);
}

void KisToolRectangleBase::updateArea() {
    const QRectF bound = createRect(m_dragStart, m_dragEnd);

    canvas()->updateCanvas(convertToPt(bound).adjusted(-100, -100, +200, +200));

    emit rectangleChanged(bound);
}

qreal KisToolRectangleBase::getRotationAngle() {
    return m_angle + m_angleBuffer;
}

QPainterPath KisToolRectangleBase::drawX(const QPointF &pt) {
    QPainterPath path;
    path.moveTo(QPointF(pt.x() - 5.0, pt.y() - 5.0)); path.lineTo(QPointF(pt.x() + 5.0, pt.y() + 5.0));
    path.moveTo(QPointF(pt.x() - 5.0, pt.y() + 5.0)); path.lineTo(QPointF(pt.x() + 5.0, pt.y() - 5.0));
    return path;
}

void KisToolRectangleBase::getRotatedPath(QPainterPath &path, const QPointF &center, const qreal &angle) {
    QTransform t;
    t.translate(center.x(), center.y());
    t.rotateRadians(angle);
    t.translate(-center.x(), -center.y());

    path = t.map(path);
}
