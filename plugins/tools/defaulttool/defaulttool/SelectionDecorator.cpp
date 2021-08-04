/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2006 Thorsten Zachmann <zachmann@kde.org>
   SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "SelectionDecorator.h"

#include <QPainterPath>

#include <KoShape.h>
#include <KoSelection.h>
#include <KoResourcePaths.h>
#include "kis_algebra_2d.h"

#include "kis_debug.h"
#include <KisHandlePainterHelper.h>
#include <KoCanvasResourceProvider.h>
#include <KisQPainterStateSaver.h>
#include "KoShapeGradientHandles.h"
#include <KoCanvasBase.h>
#include <KoSvgTextShape.h>

#include "kis_painting_tweaks.h"
#include "kis_coordinates_converter.h"
#include "kis_icon_utils.h"
#include "KisReferenceImage.h"



#define HANDLE_DISTANCE 10

SelectionDecorator::SelectionDecorator(KoCanvasResourceProvider *resourceManager)
    : m_hotPosition(KoFlake::Center)
    , m_handleRadius(7)
    , m_lineWidth(2)
    , m_showFillGradientHandles(false)
    , m_showStrokeFillGradientHandles(false)
    , m_forceShapeOutlines(false)
    , m_applyScaling(true)
{
    m_hotPosition =
        KoFlake::AnchorPosition(
            resourceManager->resource(KoFlake::HotPosition).toInt());
}

void SelectionDecorator::setSelection(KoSelection *selection)
{
    m_selection = selection;
}

void SelectionDecorator::setHandleRadius(int radius)
{
    m_handleRadius = radius;
    m_lineWidth = qMax(1, (int)(radius / 2));
}

void SelectionDecorator::setShowFillGradientHandles(bool value)
{
    m_showFillGradientHandles = value;
}

void SelectionDecorator::setShowStrokeFillGradientHandles(bool value)
{
    m_showStrokeFillGradientHandles = value;
}

void SelectionDecorator::setShowFillMeshGradientHandles(bool value)
{
    m_showFillMeshGradientHandles = value;
}

void SelectionDecorator::setCurrentMeshGradientHandles(const KoShapeMeshGradientHandles::Handle &selectedHandle,
                                                       const KoShapeMeshGradientHandles::Handle &hoveredHandle)
{
    m_selectedMeshHandle = selectedHandle;
    m_currentHoveredMeshHandle = hoveredHandle;
}

void SelectionDecorator::paint(QPainter &painter, const KoViewConverter &converter)
{
    QList<KoShape*> selectedShapes = m_selection->selectedVisibleShapes();
    if (selectedShapes.isEmpty()) return;

    const bool haveOnlyOneEditableShape =
        m_selection->selectedEditableShapes().size() == 1 &&
        selectedShapes.size() == 1;

    bool editable = false;
    bool forceBoundngRubberLine = false;

    Q_FOREACH (KoShape *shape, KoShape::linearizeSubtree(selectedShapes)) {
        if (!haveOnlyOneEditableShape || !m_showStrokeFillGradientHandles) {

            KisHandlePainterHelper helper =
                    createHandle(&painter, shape, converter);

            helper.setHandleStyle(KisHandleStyle::secondarySelection());

            if (!m_forceShapeOutlines) {
                helper.drawRubberLine(shape->outlineRect());
            } else {
                QList<QPolygonF> polys = shape->outline().toSubpathPolygons();

                if (polys.size() == 1) {
                    const QPolygonF poly1 = polys[0];
                    const QPolygonF poly2 = QPolygonF(polys[0].boundingRect());
                    const QPolygonF nonoverlap = poly2.subtracted(poly1);

                    forceBoundngRubberLine |= !nonoverlap.isEmpty();
                }

                Q_FOREACH (const QPolygonF &poly, polys) {
                    helper.drawRubberLine(poly);
                }
            }
        }

        if (shape->isShapeEditable()) {
            editable = true;
        }
    }

    const QRectF handleArea = m_selection->outlineRect();

    // draw extra rubber line around all the shapes
    if (selectedShapes.size() > 1 || forceBoundngRubberLine) {
        KisHandlePainterHelper helper =
            createHandle(&painter, m_selection, converter);

        helper.setHandleStyle(KisHandleStyle::primarySelection());
        helper.drawRubberLine(handleArea);
    }

    // if we have no editable shape selected there
    // is no need drawing the selection handles
    if (editable) {
        KisHandlePainterHelper helper =
            createHandle(&painter, m_selection, converter);
        helper.setHandleStyle(KisHandleStyle::primarySelection());

        QPolygonF outline = handleArea;

        {
            helper.drawHandleRect(outline.value(0));
            helper.drawHandleRect(outline.value(1));
            helper.drawHandleRect(outline.value(2));
            helper.drawHandleRect(outline.value(3));
            helper.drawHandleRect(0.5 * (outline.value(0) + outline.value(1)));
            helper.drawHandleRect(0.5 * (outline.value(1) + outline.value(2)));
            helper.drawHandleRect(0.5 * (outline.value(2) + outline.value(3)));
            helper.drawHandleRect(0.5 * (outline.value(3) + outline.value(0)));

            QPointF hotPos = KoFlake::anchorToPoint(m_hotPosition, handleArea);
            helper.setHandleStyle(KisHandleStyle::highlightedPrimaryHandles());
            helper.drawHandleRect(hotPos);
        }
    }

    if (haveOnlyOneEditableShape) {
        KoShape *shape = selectedShapes.first();

        if (m_showFillGradientHandles) {
            paintGradientHandles(shape, KoFlake::Fill, painter, converter);
        } else if (m_showStrokeFillGradientHandles) {
            paintGradientHandles(shape, KoFlake::StrokeFill, painter, converter);
        }

        // paint meshgradient handles
        if(m_showFillMeshGradientHandles) {
            paintMeshGradientHandles(shape, KoFlake::Fill, painter, converter);
        }
    }

}

KisHandlePainterHelper SelectionDecorator::createHandle(QPainter *painter, KoShape *shape, const KoViewConverter &converter)
{
    KisReferenceImage *reference = dynamic_cast<KisReferenceImage*>(shape);
    if(reference)
    {
        m_applyScaling = false;
    }

    if(!m_applyScaling) {
        return KoShape::createHandlePainterHelperDocument(painter, shape, m_handleRadius);
    }

    return KoShape::createHandlePainterHelperView(painter, shape, converter, m_handleRadius);
}

void SelectionDecorator::paintGradientHandles(KoShape *shape, KoFlake::FillVariant fillVariant, QPainter &painter, const KoViewConverter &converter)
{
    KoShapeGradientHandles gradientHandles(fillVariant, shape);
    QVector<KoShapeGradientHandles::Handle> handles = gradientHandles.handles();

    KisHandlePainterHelper helper =
        KoShape::createHandlePainterHelperView(&painter, shape, converter, m_handleRadius);

    const QTransform t = shape->absoluteTransformation().inverted();

    if (gradientHandles.type() == QGradient::LinearGradient) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(handles.size() == 2);

        if (handles.size() == 2) {
            helper.setHandleStyle(KisHandleStyle::gradientArrows());
            helper.drawGradientArrow(t.map(handles[0].pos), t.map(handles[1].pos), 1.5 * m_handleRadius);
        }
    }

    helper.setHandleStyle(KisHandleStyle::gradientHandles());

    Q_FOREACH (const KoShapeGradientHandles::Handle &h, handles) {
        if (h.type == KoShapeGradientHandles::Handle::RadialCenter) {
            helper.drawGradientCrossHandle(t.map(h.pos), 1.2 * m_handleRadius);
        } else {
            helper.drawGradientHandle(t.map(h.pos), 1.2 * m_handleRadius);
        }
    }
}

void SelectionDecorator::paintMeshGradientHandles(KoShape *shape,
                                                  KoFlake::FillVariant fillVariant,
                                                  QPainter &painter,
                                                  const KoViewConverter &converter)
{
    KoShapeMeshGradientHandles gradientHandles(fillVariant, shape);

    KisHandlePainterHelper helper =
        KoShape::createHandlePainterHelperView(&painter, shape, converter, m_handleRadius);
    helper.setHandleStyle(KisHandleStyle::secondarySelection());

    helper.drawPath(gradientHandles.path());

    // invert them, because we draw in logical coordinates.
    QTransform t = shape->absoluteTransformation().inverted();
    QVector<KoShapeMeshGradientHandles::Handle> cornerHandles = gradientHandles.handles();
    for (const auto& corner: cornerHandles) {
        if (corner.type == KoShapeMeshGradientHandles::Handle::BezierHandle) {
            helper.drawHandleSmallCircle(t.map(corner.pos));
        } else if (corner.type == KoShapeMeshGradientHandles::Handle::Corner) {
            helper.drawHandleRect(t.map(corner.pos));
        }
    }

    helper.setHandleStyle(KisHandleStyle::highlightedPrimaryHandlesWithSolidOutline());

    // highlight the selected handle (only corner)
    if (m_selectedMeshHandle.type == KoShapeMeshGradientHandles::Handle::Corner) {
        helper.drawHandleRect(t.map(gradientHandles.getHandle(m_selectedMeshHandle.getPosition()).pos));
    }

    // highlight the path which is being hovered/moved
    if (m_currentHoveredMeshHandle.type != KoShapeMeshGradientHandles::Handle::None) {
        QVector<QPainterPath> paths = gradientHandles.getConnectedPath(m_currentHoveredMeshHandle);
        for (const auto &path: paths) {
            helper.drawPath(path);
        }
    }
}

void SelectionDecorator::setForceShapeOutlines(bool value)
{
    m_forceShapeOutlines = value;
}

void SelectionDecorator::setReferenceImagesLayer(KisSharedPtr<KisReferenceImagesLayer> layer)
{
    m_referenceImagesLayer = layer;
}
