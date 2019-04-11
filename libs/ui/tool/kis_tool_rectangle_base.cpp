/* This file is part of the KDE project
 * Copyright (C) 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_tool_rectangle_base.h"

#include <QtCore/qmath.h>

#include <KoPointerEvent.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoViewConverter.h>
#include "kis_canvas2.h"

#include "kis_rectangle_constraint_widget.h"

KisToolRectangleBase::KisToolRectangleBase(KoCanvasBase * canvas, KisToolRectangleBase::ToolType type, const QCursor & cursor)
    : KisToolShape(canvas, cursor)
    , m_dragStart(0, 0)
    , m_dragEnd(0, 0)
    , m_type(type)
    , m_isRatioForced(false)
    , m_isWidthForced(false)
    , m_isHeightForced(false)
    , m_listenToModifiers(true)
    , m_forcedRatio(1.0)
    , m_forcedWidth(0)
    , m_forcedHeight(0)
    , m_roundCornersX(0)
    , m_roundCornersY(0)
{
}


QList<QPointer<QWidget> > KisToolRectangleBase::createOptionWidgets()
{
  QList<QPointer<QWidget> > widgetsList = KisToolShape::createOptionWidgets();

  widgetsList.append(new KisRectangleConstraintWidget(0, this, showRoundCornersGUI()));

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
  if (ratio < 0.0001f) m_isRatioForced = false;
}

void KisToolRectangleBase::roundCornersChanged(int rx, int ry)
{
    m_roundCornersX = rx;
    m_roundCornersY = ry;
}

void KisToolRectangleBase::paint(QPainter& gc, const KoViewConverter &converter)
{
    if(mode() == KisTool::PAINT_MODE) {
        paintRectangle(gc, createRect(m_dragStart, m_dragEnd));
    }

    KisToolPaint::paint(gc, converter);
}

void KisToolRectangleBase::activate(KoToolBase::ToolActivation toolActivation, const QSet<KoShape *> &shapes)
{
    KisToolShape::activate(toolActivation, shapes);

    emit sigRequestReloadConfig();
}

void KisToolRectangleBase::deactivate()
{
    updateArea();
    KisToolShape::deactivate();
}

void KisToolRectangleBase::listenToModifiers(bool listen)
{
    m_listenToModifiers = listen;
}
bool KisToolRectangleBase::listeningToModifiers()
{
    return m_listenToModifiers;
}

void KisToolRectangleBase::beginPrimaryAction(KoPointerEvent *event)
{
    if ((m_type == PAINT && (!nodeEditable() || nodePaintAbility() == UNPAINTABLE)) ||
        (m_type == SELECT && !selectionEditable())) {

        event->ignore();
        return;
    }
    setMode(KisTool::PAINT_MODE);

    QPointF pos = convertToPixelCoordAndSnap(event, QPointF(), false);
    m_dragStart = m_dragCenter = pos;

    QSizeF area = QSizeF(0,0);

    applyConstraints(area, false);

    m_dragEnd.setX(m_dragStart.x() + area.width());
    m_dragEnd.setY(m_dragStart.y() + area.height());

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

    bool constraintToggle = (event->modifiers() & Qt::ShiftModifier) && m_listenToModifiers;
    bool translateMode = (event->modifiers() & Qt::AltModifier) && m_listenToModifiers;
    bool expandFromCenter = (event->modifiers() & Qt::ControlModifier) && m_listenToModifiers;

    bool fixedSize = isFixedSize() && !constraintToggle;

    QPointF pos = convertToPixelCoordAndSnap(event, QPointF(), false);

    if (fixedSize) {
      m_dragStart = pos;
    } else if (translateMode) {
      QPointF trans = pos - m_dragEnd;
      m_dragStart += trans;
      m_dragEnd += trans;
    }

    QPointF diag = pos - m_dragStart;
    QSizeF area = QSizeF(fabs(diag.x()), fabs(diag.y()));

    bool overrideRatio = constraintToggle && !(m_isHeightForced || m_isWidthForced || m_isRatioForced);
    if (!constraintToggle || overrideRatio) {
      applyConstraints(area, overrideRatio);
    }

    diag = QPointF(
      (diag.x() < 0) ? -area.width() : area.width(),
      (diag.y() < 0) ? -area.height() : area.height()
    );

    // resize around center point?
    if (expandFromCenter && !fixedSize) {
      m_dragStart = m_dragCenter - diag / 2;
      m_dragEnd = m_dragCenter + diag / 2;
    } else {
      m_dragEnd = m_dragStart + diag;
    }

    updateArea();

    m_dragCenter = QPointF((m_dragStart.x() + m_dragEnd.x()) / 2,
                           (m_dragStart.y() + m_dragEnd.y()) / 2);
    KisToolPaint::requestUpdateOutline(event->point, event);
}

void KisToolRectangleBase::endPrimaryAction(KoPointerEvent *event)
{
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);
    setMode(KisTool::HOVER_MODE);

    updateArea();

    finishRect(createRect(m_dragStart, m_dragEnd), m_roundCornersX, m_roundCornersY);
    event->accept();
}

QRectF KisToolRectangleBase::createRect(const QPointF &start, const QPointF &end)
{
    /**
     * To make the dragging user-friendly it should work in a bit
     * non-obvious way: the start-drag point must be handled with
     * "ceil"/"floor" (depending on the direction of the drag) and the
     * end-drag point should follow usual "round" semantics.
     */

    qreal x0 = start.x();
    qreal y0 = start.y();
    qreal x1 = end.x();
    qreal y1 = end.y();

    int newX0 = qRound(x0);
    int newY0 = qRound(y0);

    int newX1 = qRound(x1);
    int newY1 = qRound(y1);

    QRectF result;
    result.setCoords(newX0, newY0, newX1, newY1);
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
    paintToolOutline(&gc, path);
}

void KisToolRectangleBase::updateArea() {
    const QRectF bound = createRect(m_dragStart, m_dragEnd);

    canvas()->updateCanvas(convertToPt(bound).adjusted(-100, -100, +200, +200));

    emit rectangleChanged(bound);
}

