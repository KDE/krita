/* This file is part of the KDE project
 * Copyright (C) 2006,2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007,2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "KoPathToolHandle.h"
#include "KoPathTool.h"
#include "KoPathPointMoveStrategy.h"
#include "KoPathControlPointMoveStrategy.h"
#include "KoPathConnectionPointStrategy.h"
#include "KoSelection.h"
#include "commands/KoPathPointTypeCommand.h"
#include "KoParameterChangeStrategy.h"
#include "KoParameterShape.h"
#include "KoCanvasBase.h"
#include "KoDocumentResourceManager.h"
#include "KoConnectionShape.h"
#include "KoViewConverter.h"
#include "KoPointerEvent.h"
#include "KoShapeController.h"
#include <QPainter>
#include <KisHandlePainterHelper.h>


KoPathToolHandle::KoPathToolHandle(KoPathTool *tool)
        : m_tool(tool)
{
}

KoPathToolHandle::~KoPathToolHandle()
{
}

uint KoPathToolHandle::handleRadius() const
{
    return m_tool->canvas()->shapeController()->resourceManager()->handleRadius();
}

PointHandle::PointHandle(KoPathTool *tool, KoPathPoint *activePoint, KoPathPoint::PointType activePointType)
        : KoPathToolHandle(tool)
        , m_activePoint(activePoint)
        , m_activePointType(activePointType)
{
}

void PointHandle::paint(QPainter &painter, const KoViewConverter &converter, qreal handleRadius)
{
    KoPathToolSelection * selection = dynamic_cast<KoPathToolSelection*>(m_tool->selection());

    KoPathPoint::PointTypes allPaintedTypes = KoPathPoint::Node;
    if (selection && selection->contains(m_activePoint)) {
        allPaintedTypes = KoPathPoint::All;
    }


    KisHandlePainterHelper helper = KoShape::createHandlePainterHelperView(&painter, m_activePoint->parent(), converter, handleRadius);


    if (allPaintedTypes != m_activePointType) {
        KoPathPoint::PointTypes nonHighlightedType = allPaintedTypes & ~m_activePointType;
        KoPathPoint::PointTypes nonNodeType = nonHighlightedType & ~KoPathPoint::Node;

        if (nonNodeType != KoPathPoint::None) {
            helper.setHandleStyle(KisHandleStyle::selectedPrimaryHandles());
            m_activePoint->paint(helper, nonHighlightedType);
        }

        if (nonHighlightedType & KoPathPoint::Node) {
            helper.setHandleStyle(KisHandleStyle::partiallyHighlightedPrimaryHandles());
            m_activePoint->paint(helper, KoPathPoint::Node);
        }
    }

    helper.setHandleStyle(KisHandleStyle::highlightedPrimaryHandles());
    m_activePoint->paint(helper, m_activePointType);
}

void PointHandle::repaint() const
{
    m_tool->repaint(m_oldRepaintedRect);
    bool active = false;
    KoPathToolSelection * selection = dynamic_cast<KoPathToolSelection*>(m_tool->selection());
    if (selection && selection->contains(m_activePoint))
        active = true;
    m_oldRepaintedRect = m_activePoint->boundingRect(!active);
    m_tool->repaint(m_oldRepaintedRect);
}

KoInteractionStrategy * PointHandle::handleMousePress(KoPointerEvent *event)
{
    if ((event->button() & Qt::LeftButton) == 0)
        return 0;
    if ((event->modifiers() & Qt::ControlModifier) == 0) { // no shift pressed.
        KoPathToolSelection * selection = dynamic_cast<KoPathToolSelection*>(m_tool->selection());

        // control select adds/removes points to/from the selection
        if (event->modifiers() & Qt::ShiftModifier) {
            if (selection->contains(m_activePoint)) {
                selection->remove(m_activePoint);
            } else {
                selection->add(m_activePoint, false);
            }
            m_tool->repaint(m_activePoint->boundingRect(false));
        } else {
            // no control modifier, so clear selection and select active point
            if (!selection->contains(m_activePoint)) {
                selection->add(m_activePoint, true);
                m_tool->repaint(m_activePoint->boundingRect(false));
            }
        }
        // TODO remove canvas from call ?
        if (m_activePointType == KoPathPoint::Node) {
            QPointF startPoint = m_activePoint->parent()->shapeToDocument(m_activePoint->point());
            return new KoPathPointMoveStrategy(m_tool, startPoint);
        } else {
            KoPathShape * pathShape = m_activePoint->parent();
            KoPathPointData pd(pathShape, pathShape->pathPointIndex(m_activePoint));
            return new KoPathControlPointMoveStrategy(m_tool, pd, m_activePointType, event->point);
        }
    } else {
        KoPathPoint::PointProperties props = m_activePoint->properties();
        if (! m_activePoint->activeControlPoint1() || ! m_activePoint->activeControlPoint2())
            return 0;

        KoPathPointTypeCommand::PointType pointType = KoPathPointTypeCommand::Smooth;
        // cycle the smooth->symmetric->unsmooth state of the path point
        if (props & KoPathPoint::IsSmooth)
            pointType = KoPathPointTypeCommand::Symmetric;
        else if (props & KoPathPoint::IsSymmetric)
            pointType = KoPathPointTypeCommand::Corner;

        QList<KoPathPointData> pointData;
        pointData.append(KoPathPointData(m_activePoint->parent(), m_activePoint->parent()->pathPointIndex(m_activePoint)));
        m_tool->canvas()->addCommand(new KoPathPointTypeCommand(pointData, pointType));
    }
    return 0;
}

bool PointHandle::check(const QList<KoPathShape*> &selectedShapes)
{
    if (selectedShapes.contains(m_activePoint->parent())) {
        return m_activePoint->parent()->pathPointIndex(m_activePoint) != KoPathPointIndex(-1, -1);
    }
    return false;
}

KoPathPoint * PointHandle::activePoint() const
{
    return m_activePoint;
}

KoPathPoint::PointType PointHandle::activePointType() const
{
    return m_activePointType;
}

void PointHandle::trySelectHandle()
{
    KoPathToolSelection * selection = dynamic_cast<KoPathToolSelection*>(m_tool->selection());

    if (!selection->contains(m_activePoint) && m_activePointType == KoPathPoint::Node) {
        selection->clear();
        selection->add(m_activePoint, false);
    }
}

ParameterHandle::ParameterHandle(KoPathTool *tool, KoParameterShape *parameterShape, int handleId)
        : KoPathToolHandle(tool)
        , m_parameterShape(parameterShape)
        , m_handleId(handleId)
{
}

void ParameterHandle::paint(QPainter &painter, const KoViewConverter &converter, qreal handleRadius)
{
    KisHandlePainterHelper helper = KoShape::createHandlePainterHelperView(&painter, m_parameterShape, converter, handleRadius);
    helper.setHandleStyle(KisHandleStyle::highlightedPrimaryHandles());
    m_parameterShape->paintHandle(helper, m_handleId);
}

void ParameterHandle::repaint() const
{
    m_tool->repaint(m_parameterShape->shapeToDocument(QRectF(m_parameterShape->handlePosition(m_handleId), QSize(1, 1))));
}

KoInteractionStrategy * ParameterHandle::handleMousePress(KoPointerEvent *event)
{
    if (event->button() & Qt::LeftButton) {
        KoPathToolSelection * selection = dynamic_cast<KoPathToolSelection*>(m_tool->selection());
        if (selection)
            selection->clear();
        return new KoParameterChangeStrategy(m_tool, m_parameterShape, m_handleId);
    }
    return 0;
}

bool ParameterHandle::check(const QList<KoPathShape*> &selectedShapes)
{
    return selectedShapes.contains(m_parameterShape);
}


ConnectionHandle::ConnectionHandle(KoPathTool *tool, KoParameterShape *parameterShape, int handleId)
        : ParameterHandle(tool, parameterShape, handleId)
{
}

KoInteractionStrategy * ConnectionHandle::handleMousePress(KoPointerEvent *event)
{
    if (event->button() & Qt::LeftButton) {
        KoPathToolSelection * selection = dynamic_cast<KoPathToolSelection*>(m_tool->selection());
        if (selection)
            selection->clear();
        KoConnectionShape * shape = dynamic_cast<KoConnectionShape*>(m_parameterShape);
        if (! shape)
            return 0;
        return new KoPathConnectionPointStrategy(m_tool, shape, m_handleId);
    }
    return 0;
}
