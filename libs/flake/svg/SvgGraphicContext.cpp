/* This file is part of the KDE project
 * Copyright (C) 2003,2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2007,2009 Jan Hambrecht <jaham@gmx.net>
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

#include "SvgGraphicContext.h"

#include "kis_pointer_utils.h"


SvgGraphicsContext::SvgGraphicsContext()
: stroke(toQShared(new KoShapeStroke()))
, textProperties(KoSvgTextProperties::defaultProperties())
{
    stroke->setLineStyle(Qt::NoPen, QVector<qreal>());   // default is no stroke
    stroke->setLineWidth(1.0);
    stroke->setCapStyle(Qt::FlatCap);
    stroke->setJoinStyle(Qt::MiterJoin);
}

SvgGraphicsContext::SvgGraphicsContext(const SvgGraphicsContext &gc)
    : stroke(toQShared(new KoShapeStroke(*(gc.stroke.data()))))
{
    KoShapeStrokeSP newStroke = stroke;
    *this = gc;
    this->stroke = newStroke;
}

void SvgGraphicsContext::workaroundClearInheritedFillProperties()
{
    /**
     * HACK ALERT: according to SVG patterns, clip paths and clip masks
     *             must not inherit any properties from the referencing element.
     *             We still don't support it, therefore we reset only fill/stroke
     *             properties to avoid cyclic fill inheritance, which may cause
     *             infinite recursion.
     */


    strokeType = None;

    stroke = toQShared(new KoShapeStroke());
    stroke->setLineStyle(Qt::NoPen, QVector<qreal>());   // default is no stroke
    stroke->setLineWidth(1.0);
    stroke->setCapStyle(Qt::FlatCap);
    stroke->setJoinStyle(Qt::MiterJoin);

    fillType = Solid;
    fillRule = Qt::WindingFill;
    fillColor = QColor(Qt::black);   // default is black fill as per svg spec

    opacity = 1.0;

    currentColor = Qt::black;
}
