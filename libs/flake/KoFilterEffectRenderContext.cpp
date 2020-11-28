/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "KoFilterEffectRenderContext.h"
#include "KoViewConverter.h"

#include <QRectF>
#include <QTransform>

class Q_DECL_HIDDEN KoFilterEffectRenderContext::Private
{
public:
    Private(const KoViewConverter &viewConverter)
        : converter(viewConverter)
    {}

    QRectF filterRegion;
    QRectF shapeBound;
    const KoViewConverter & converter;
};

KoFilterEffectRenderContext::KoFilterEffectRenderContext(const KoViewConverter &converter)
: d(new Private(converter))
{
}

KoFilterEffectRenderContext::~KoFilterEffectRenderContext()
{
    delete d;
}

QRectF KoFilterEffectRenderContext::filterRegion() const
{
    return d->filterRegion;
}

void KoFilterEffectRenderContext::setFilterRegion(const QRectF &filterRegion)
{
    d->filterRegion = filterRegion;
}

void KoFilterEffectRenderContext::setShapeBoundingBox(const QRectF &bound)
{
    d->shapeBound = bound;
}

QPointF KoFilterEffectRenderContext::toUserSpace(const QPointF &value) const
{
    return QPointF(value.x()*d->shapeBound.width(), value.y()*d->shapeBound.height());
}

qreal KoFilterEffectRenderContext::toUserSpaceX(qreal value) const
{
    return value * d->shapeBound.width();
}

qreal KoFilterEffectRenderContext::toUserSpaceY(qreal value) const
{
    return value * d->shapeBound.height();
}

const KoViewConverter * KoFilterEffectRenderContext::viewConverter() const
{
    return &d->converter;
}
