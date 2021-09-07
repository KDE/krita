/*
 *  SPDX-FileCopyrightText: 2021 Sachin Jindal <jindalsachin01@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <QPainter>

#include "KisReferenceImageSelectionDecorator.h"
#include <KoShape.h>
#include <KoViewConverter.h>
#include "kis_canvas2.h"
#include "KisHandlePainterHelper.h"

KisReferenceImageSelectionDecorator::KisReferenceImageSelectionDecorator()
    : m_handleRadius(5)
    , m_hotPosition(KoFlake::Center)
{
}

void KisReferenceImageSelectionDecorator::setSelection(KoSelection *selection)
{
    m_selection = selection;
}

void KisReferenceImageSelectionDecorator::drawHandle(QPainter &gc, KoShape *shape, QRectF handleArea, bool paintHotPosition)
{
    KisHandlePainterHelper helper =
    KoShape::createHandlePainterHelperDocument(&gc, shape, m_handleRadius);
    helper.setHandleStyle(KisHandleStyle::primarySelection());

    QPolygonF outline = handleArea;
    helper.drawHandleRect(outline.value(0));
    helper.drawHandleRect(outline.value(1));
    helper.drawHandleRect(outline.value(2));
    helper.drawHandleRect(outline.value(3));
    helper.drawHandleRect(0.5 * (outline.value(0) + outline.value(1)));
    helper.drawHandleRect(0.5 * (outline.value(1) + outline.value(2)));
    helper.drawHandleRect(0.5 * (outline.value(2) + outline.value(3)));
    helper.drawHandleRect(0.5 * (outline.value(3) + outline.value(0)));

    if (paintHotPosition) {
        QPointF hotPos = KoFlake::anchorToPoint(m_hotPosition, handleArea);
        helper.setHandleStyle(KisHandleStyle::highlightedPrimaryHandles());
        helper.drawHandleRect(hotPos);
    }
}

void KisReferenceImageSelectionDecorator::paint(QPainter &gc)
{
    if (!m_selection) return;

    QList<KoShape*> selectedShapes = m_selection->selectedVisibleShapes();
    if (selectedShapes.isEmpty()) return;

    KisReferenceImage *referenceImage = nullptr;
    if (selectedShapes.size() == 1) {
        referenceImage = dynamic_cast<KisReferenceImage*>(selectedShapes.at(0));
    }
    if (referenceImage && referenceImage->cropEnabled()) {
        QRectF cropBorderRect = referenceImage->cropRect();
        QTransform transform = referenceImage->absoluteTransformation();
        QPolygonF shapeBounds = transform.map(QPolygonF(referenceImage->outlineRect()));
        QPolygonF m_cropBorder = transform.map(QPolygonF(cropBorderRect));

        QPainterPath path;
        path.addPolygon(shapeBounds);
        path.addPolygon(m_cropBorder);
        gc.setPen(Qt::NoPen);
        gc.setBrush(QColor(0, 0, 0, 200));
        gc.drawPath(path);

        drawHandle(gc, referenceImage, referenceImage->cropRect());
        return;
    }

    bool editable = false;
    Q_FOREACH (KoShape *shape, KoShape::linearizeSubtree(selectedShapes)) {
        KisHandlePainterHelper helper =
                KoShape::createHandlePainterHelperDocument(&gc, shape, m_handleRadius);

                helper.setHandleStyle(KisHandleStyle::secondarySelection());
                helper.drawRubberLine(shape->outlineRect());

        if (shape->isShapeEditable()) {
            editable = true;
        }
    }

    const QRectF handleArea = m_selection->outlineRect();

    // draw extra rubber line around all the shapes
    if (selectedShapes.size() > 1) {
        KisHandlePainterHelper helper =
        KoShape::createHandlePainterHelperDocument(&gc, m_selection, m_handleRadius);

        helper.setHandleStyle(KisHandleStyle::primarySelection());
        helper.drawRubberLine(handleArea);
    }

    // if we have no editable shape selected there
    // is no need drawing the selection handles
    if (editable) {
        drawHandle(gc, m_selection, handleArea, true);
    }
}
