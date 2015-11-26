/* This file is part of the KDE project
* Copyright (c) 2010 Jan Hambrecht <jaham@gmx.net>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; see the file COPYING.LIB.  If not, write to
* the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
*/

#include "FilterRegionEditStrategy.h"
#include "FilterRegionChangeCommand.h"
#include <KoShape.h>
#include <KoFilterEffect.h>
#include <KoViewConverter.h>
#include <QPainter>

FilterRegionEditStrategy::FilterRegionEditStrategy(KoToolBase *parent, KoShape *shape, KoFilterEffect *effect, KarbonFilterEffectsTool::EditMode mode)
    : KoInteractionStrategy(parent)
    , m_effect(effect)
    , m_shape(shape)
    , m_editMode(mode)
{
    Q_ASSERT(m_effect);
    Q_ASSERT(m_shape);
    // get the size rect of the shape
    m_sizeRect = QRectF(QPointF(), m_shape->size());
    // get the filter rectangle in shape coordinates
    m_filterRect = m_effect->filterRectForBoundingRect(m_sizeRect);
}

void FilterRegionEditStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
    QPointF shapePoint = m_shape->documentToShape(mouseLocation);
    if (m_lastPosition.isNull()) {
        m_lastPosition = shapePoint;
    }
    QPointF delta = shapePoint - m_lastPosition;
    if (delta.isNull()) {
        return;
    }

    switch (m_editMode) {
    case KarbonFilterEffectsTool::MoveAll:
        m_filterRect.translate(delta.x(), delta.y());
        break;
    case KarbonFilterEffectsTool::MoveLeft:
        m_filterRect.setLeft(m_filterRect.left() + delta.x());
        break;
    case KarbonFilterEffectsTool::MoveRight:
        m_filterRect.setRight(m_filterRect.right() + delta.x());
        break;
    case KarbonFilterEffectsTool::MoveTop:
        m_filterRect.setTop(m_filterRect.top() + delta.y());
        break;
    case KarbonFilterEffectsTool::MoveBottom:
        m_filterRect.setBottom(m_filterRect.bottom() + delta.y());
        break;
    default:
        // nothing to do here
        return;
    }
    tool()->repaintDecorations();
    m_lastPosition = shapePoint;
}

KUndo2Command *FilterRegionEditStrategy::createCommand()
{
    qreal x = m_filterRect.left() / m_sizeRect.width();
    qreal y = m_filterRect.top() / m_sizeRect.height();
    qreal w = m_filterRect.width() / m_sizeRect.width();
    qreal h = m_filterRect.height() / m_sizeRect.height();
    return new FilterRegionChangeCommand(m_effect, QRectF(x, y, w, h), m_shape);
}

void FilterRegionEditStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
}

void FilterRegionEditStrategy::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(converter);
    // paint the filter subregion rect
    painter.setBrush(Qt::NoBrush);
    painter.setPen(Qt::red);
    painter.drawRect(m_filterRect);
}
