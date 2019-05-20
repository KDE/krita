/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#include "KoColorBackground.h"
#include "KoColorBackground_p.h"
#include "KoShapeSavingContext.h"
#include <KoOdfGraphicStyles.h>
#include <KoOdfLoadingContext.h>
#include <KoXmlNS.h>
#include <KoStyleStack.h>

#include <QColor>
#include <QPainter>

KoColorBackground::KoColorBackground()
: KoShapeBackground(*(new KoColorBackgroundPrivate()))
{
}

KoColorBackground::KoColorBackground(KoShapeBackgroundPrivate &dd)
: KoShapeBackground(dd)
{
}

KoColorBackground::KoColorBackground(const QColor &color, Qt::BrushStyle style)
: KoShapeBackground(*(new KoColorBackgroundPrivate()))
{
    SHARED_D(KoColorBackground);
    if (style < Qt::SolidPattern || style >= Qt::LinearGradientPattern)
        style = Qt::SolidPattern;
    d->style = style;
    d->color = color;
}

KoColorBackground::~KoColorBackground()
{
}

bool KoColorBackground::compareTo(const KoShapeBackground *other) const
{
    CONST_SHARED_D(KoColorBackground);

    const KoColorBackground *bg = dynamic_cast<const KoColorBackground*>(other);
    return bg && bg->color() == d->color;
}

QColor KoColorBackground::color() const
{
    CONST_SHARED_D(KoColorBackground);
    return d->color;
}

void KoColorBackground::setColor(const QColor &color)
{
    SHARED_D(KoColorBackground);
    d->color = color;
}

Qt::BrushStyle KoColorBackground::style() const
{
    CONST_SHARED_D(KoColorBackground);
    return d->style;
}

QBrush KoColorBackground::brush() const
{
    CONST_SHARED_D(KoColorBackground);
    return QBrush(d->color, d->style);
}

void KoColorBackground::paint(QPainter &painter, const KoViewConverter &/*converter*/, KoShapePaintingContext &/*context*/, const QPainterPath &fillPath) const
{
    painter.setBrush(brush());
    painter.drawPath(fillPath);
}

void KoColorBackground::fillStyle(KoGenStyle &style, KoShapeSavingContext &context)
{
    SHARED_D(KoColorBackground);
    KoOdfGraphicStyles::saveOdfFillStyle(style, context.mainStyles(), QBrush(d->color, d->style));
}

bool KoColorBackground::loadStyle(KoOdfLoadingContext & context, const QSizeF &)
{
    SHARED_D(KoColorBackground);
    KoStyleStack &styleStack = context.styleStack();
    if (! styleStack.hasProperty(KoXmlNS::draw, "fill"))
        return false;

    QString fillStyle = styleStack.property(KoXmlNS::draw, "fill");
    if (fillStyle == "solid" || fillStyle == "hatch") {
        QBrush brush = KoOdfGraphicStyles::loadOdfFillStyle(styleStack, fillStyle, context.stylesReader());
        d->color = brush.color();
        d->style = brush.style();
        return true;
    }

    return false;
}
