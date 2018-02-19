/* This file is part of the KDE project

   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "SelectionDecorator.h"

#include <KoShape.h>
#include <KoSelection.h>
#include <KoResourcePaths.h>
#include "kis_algebra_2d.h"

#include "kis_debug.h"
#include <KisHandlePainterHelper.h>
#include <KoShapeHandlesCollection.h>
#include <KoCanvasResourceManager.h>
#include <KisQPainterStateSaver.h>
#include "KoShapeGradientHandles.h"

#include "kis_painting_tweaks.h"

#define HANDLE_DISTANCE 10

SelectionDecorator::SelectionDecorator(KoCanvasResourceManager *resourceManager)
    : m_hotPosition(KoFlake::Center)
    , m_handleRadius(7)
    , m_lineWidth(2)
    , m_showFillGradientHandles(false)
    , m_showStrokeFillGradientHandles(false)
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

void SelectionDecorator::paint(QPainter &painter, const KoViewConverter &converter)
{
    KoShapeHandlesCollection collection;
    collection.addHandles(calculateCurrentHandles());
    collection.drawHandles(&painter, converter, m_handleRadius);
}

QVector<QRectF> SelectionDecorator::updateDocRects(const KoViewConverter &converter) const
{
    KoShapeHandlesCollection collection;
    collection.addHandles(calculateCurrentHandles());
    return collection.updateDocRects(converter, m_handleRadius);
}

QVector<KoFlake::HandlesRecord> SelectionDecorator::calculateGradientHandles(KoShape *shape, KoFlake::FillVariant fillVariant) const
{
    using KoFlake::Handle;
    using KoFlake::HandlesRecord;
    using KoFlake::HandlePointType;
    using KoFlake::HandleLineType;

    QVector<HandlesRecord> result;

    KoShapeGradientHandles gradientHandles(fillVariant, shape);
    QVector<KoShapeGradientHandles::Handle> handles = gradientHandles.handles();

    const QTransform t = shape->absoluteTransformation(0).inverted();

    if (gradientHandles.type() == QGradient::LinearGradient) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(handles.size() == 2);
        if (handles.size() == 2) {

            result << HandlesRecord(shape, KisHandleStyle::gradientArrows(),
                                    Handle(HandleLineType::GradientArrow, t.map(handles[0].pos), t.map(handles[1].pos)));
        }
    }

    Q_FOREACH (const KoShapeGradientHandles::Handle &h, handles) {
        if (h.type == KoShapeGradientHandles::Handle::RadialCenter) {
            result << HandlesRecord(shape, KisHandleStyle::gradientHandles(),
                                    Handle(HandlePointType::GradientCross, t.map(h.pos)));
        } else {
            result << HandlesRecord(shape, KisHandleStyle::gradientHandles(),
                                    Handle(HandlePointType::GradientDiamond, t.map(h.pos)));
        }
    }

    return result;
}

QVector<KoFlake::HandlesRecord> SelectionDecorator::calculateCurrentHandles() const
{
    using KoFlake::Handle;
    using KoFlake::HandlesRecord;
    using KoFlake::HandlePointType;
    using KoFlake::HandlePathType;

    QVector<HandlesRecord> result;

    QList<KoShape*> selectedShapes = m_selection->selectedVisibleShapes();
    if (selectedShapes.isEmpty()) return result;

    const bool haveOnlyOneEditableShape =
        m_selection->selectedEditableShapes().size() == 1 &&
        selectedShapes.size() == 1;

    bool editable = false;

    Q_FOREACH (KoShape *shape, KoShape::linearizeSubtree(selectedShapes)) {
        if (!haveOnlyOneEditableShape || !m_showStrokeFillGradientHandles) {
            result << HandlesRecord(shape,
                                    KisHandleStyle::secondarySelection(),
                                    Handle(HandlePathType::OutlinePath, shape->outlineRect()));
        }

        if (shape->isShapeEditable()) {
            editable = true;
        }
    }

    const QRectF handleArea = m_selection->outlineRect();

    // draw extra rubber line around all the shapes
    if (selectedShapes.size() > 1) {
        result << HandlesRecord(m_selection,
                                KisHandleStyle::primarySelection(),
                                Handle(HandlePathType::OutlinePath, handleArea));
    }

    // if we have no editable shape selected there
    // is no need drawing the selection handles
    if (editable) {

        {
            QPolygonF outline = handleArea;
            QVector<Handle> handles;

            handles << Handle(HandlePointType::Rect, outline.value(0));
            handles << Handle(HandlePointType::Rect, outline.value(1));
            handles << Handle(HandlePointType::Rect, outline.value(2));
            handles << Handle(HandlePointType::Rect, outline.value(3));

            handles << Handle(HandlePointType::Rect, 0.5 * (outline.value(0) + outline.value(1)));
            handles << Handle(HandlePointType::Rect, 0.5 * (outline.value(1) + outline.value(2)));
            handles << Handle(HandlePointType::Rect, 0.5 * (outline.value(2) + outline.value(3)));
            handles << Handle(HandlePointType::Rect, 0.5 * (outline.value(3) + outline.value(0)));

            result << HandlesRecord(m_selection, KisHandleStyle::primarySelection(), handles);
        }

        {
            QPointF hotPos = KoFlake::anchorToPoint(m_hotPosition, handleArea);
            result << HandlesRecord(m_selection,
                                    KisHandleStyle::highlightedPrimaryHandles(),
                                    Handle(HandlePointType::Rect, hotPos));
        }
    }

    if (haveOnlyOneEditableShape) {
        KoShape *shape = selectedShapes.first();

        if (m_showFillGradientHandles) {
            result << calculateGradientHandles(shape, KoFlake::Fill);
        } else if (m_showStrokeFillGradientHandles) {
            result << calculateGradientHandles(shape, KoFlake::StrokeFill);
        }
    }

    return result;
}
