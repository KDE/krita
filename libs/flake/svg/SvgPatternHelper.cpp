/* This file is part of the KDE project
 * Copyright (C) 2009 Jan Hambrecht <jaham@gmx.net>
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

#include "SvgPatternHelper.h"
#include "SvgUtil.h"

#include <KoViewConverter.h>
#include <KoShapePainter.h>
#include <KoShape.h>

#include <QPainter>

SvgPatternHelper::SvgPatternHelper()
        : m_patternUnits(ObjectBoundingBox), m_patternContentUnits(UserSpaceOnUse)
{
}

SvgPatternHelper::~SvgPatternHelper()
{
}

void SvgPatternHelper::setPatternUnits(Units units)
{
    m_patternUnits = units;
}

SvgPatternHelper::Units SvgPatternHelper::patternUnits() const
{
    return m_patternUnits;
}

void SvgPatternHelper::setPatternContentUnits(Units units)
{
    m_patternContentUnits = units;
}

SvgPatternHelper::Units SvgPatternHelper::patternContentUnits() const
{
    return m_patternContentUnits;
}

void SvgPatternHelper::setTransform(const QTransform &transform)
{
    m_transform = transform;
}

QTransform SvgPatternHelper::transform() const
{
    return m_transform;
}

void SvgPatternHelper::setPosition(const QPointF & position)
{
    m_position = position;
}

QPointF SvgPatternHelper::position(const QRectF & objectBound) const
{
    if (m_patternUnits == UserSpaceOnUse) {
        return m_position;
    } else {
        return SvgUtil::objectToUserSpace(m_position, objectBound);
    }
}

void SvgPatternHelper::setSize(const QSizeF & size)
{
    m_size = size;
}

QSizeF SvgPatternHelper::size(const QRectF & objectBound) const
{
    if (m_patternUnits == UserSpaceOnUse) {
        return m_size;
    } else {
        return SvgUtil::objectToUserSpace(m_size, objectBound);
    }
}

void SvgPatternHelper::setContent(const KoXmlElement &content)
{
    m_patternContent = content;
}

KoXmlElement SvgPatternHelper::content() const
{
    return m_patternContent;
}

void SvgPatternHelper::copyContent(const SvgPatternHelper &other)
{
    m_patternContent = other.m_patternContent;
}

void SvgPatternHelper::setPatternContentViewbox(const QRectF &viewBox)
{
    m_patternContentViewbox = viewBox;
}

QImage SvgPatternHelper::generateImage(const QRectF &objectBound, const QList<KoShape*> content)
{
    KoViewConverter zoomHandler;

    QSizeF patternSize = size(objectBound);
    if (patternSize.isEmpty())
        return QImage();

    QSizeF tileSize = zoomHandler.documentToView(patternSize);
    if (tileSize.isEmpty())
        return QImage();

    QTransform viewMatrix;

    if (! m_patternContentViewbox.isNull()) {
        viewMatrix.translate(-m_patternContentViewbox.x(), -m_patternContentViewbox.y());
        const qreal xScale = patternSize.width() / m_patternContentViewbox.width();
        const qreal yScale = patternSize.height() / m_patternContentViewbox.height();
        viewMatrix.scale(xScale, yScale);
    }

    // setup the tile image
    QImage tile(tileSize.toSize(), QImage::Format_ARGB32);
    tile.fill(QColor(Qt::transparent).rgba());

    // setup the painter to paint the tile content
    QPainter tilePainter(&tile);
    tilePainter.setClipRect(tile.rect());
    tilePainter.setWorldTransform(viewMatrix);
    //tilePainter.setRenderHint(QPainter::Antialiasing);

    // paint the content into the tile image
    KoShapePainter shapePainter;
    shapePainter.setShapes(content);
    shapePainter.paint(tilePainter, zoomHandler);

    return tile;
}
