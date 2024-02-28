/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2003, 2005 Rob Buis <buis@kde.org>
 * SPDX-FileCopyrightText: 2007, 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
