/* This file is part of the KDE project
 * Copyright (c) 2009-2010 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#include "KoFilterEffectStack.h"
#include "KoFilterEffect.h"
#include "KoXmlWriter.h"

#include <QtCore/QAtomicInt>
#include <QtCore/QSet>
#include <QDebug>

class KoFilterEffectStack::Private
{
public:
    Private()
    : clipRect(-0.1, -0.1, 1.2, 1.2) // initialize as per svg spec
    {
    }

    ~Private()
    {
        qDeleteAll(filterEffects);
    }

    QList<KoFilterEffect*> filterEffects;
    QRectF clipRect;
    QAtomicInt refCount;
};

KoFilterEffectStack::KoFilterEffectStack()
: d(new Private())
{
}

KoFilterEffectStack::~KoFilterEffectStack()
{
    delete d;
}

QList<KoFilterEffect*> KoFilterEffectStack::filterEffects() const
{
    return d->filterEffects;
}

bool KoFilterEffectStack::isEmpty() const
{
    return d->filterEffects.isEmpty();
}

void KoFilterEffectStack::insertFilterEffect(int index, KoFilterEffect * filter)
{
    if (filter)
        d->filterEffects.insert(index, filter);
}

void KoFilterEffectStack::appendFilterEffect(KoFilterEffect *filter)
{
    if (filter)
        d->filterEffects.append(filter);
}

void KoFilterEffectStack::removeFilterEffect(int index)
{
    KoFilterEffect * filter = takeFilterEffect(index);
    delete filter;
}

KoFilterEffect* KoFilterEffectStack::takeFilterEffect(int index)
{
    if (index >= d->filterEffects.size())
        return 0;
    return d->filterEffects.takeAt(index);
}

void KoFilterEffectStack::setClipRect(const QRectF &clipRect)
{
    d->clipRect = clipRect;
}

QRectF KoFilterEffectStack::clipRect() const
{
    return d->clipRect;
}

QRectF KoFilterEffectStack::clipRectForBoundingRect(const QRectF &boundingRect) const
{
    qreal x = boundingRect.x() + d->clipRect.x() * boundingRect.width();
    qreal y = boundingRect.y() + d->clipRect.y() * boundingRect.height();
    qreal w = d->clipRect.width() * boundingRect.width();
    qreal h = d->clipRect.height() * boundingRect.height();
    return QRectF(x, y, w, h);
}

bool KoFilterEffectStack::ref()
{
    return d->refCount.ref();
}

bool KoFilterEffectStack::deref()
{
    return d->refCount.deref();
}

int KoFilterEffectStack::useCount() const
{
    return d->refCount;
}

void KoFilterEffectStack::save(KoXmlWriter &writer, const QString &filterId)
{
    writer.startElement("filter");
    writer.addAttribute("id", filterId);
    writer.addAttribute("filterUnits", "objectBoundingBox");
    writer.addAttribute("primitiveUnits", "objectBoundingBox");
    writer.addAttribute("x", d->clipRect.x());
    writer.addAttribute("y", d->clipRect.y());
    writer.addAttribute("width", d->clipRect.width());
    writer.addAttribute("height", d->clipRect.height());

    foreach(KoFilterEffect *effect, d->filterEffects) {
        effect->save(writer);
    }

    writer.endElement();
}

QSet<QString> KoFilterEffectStack::requiredStandarsInputs() const
{
    static QSet<QString> stdInputs = QSet<QString>()
        << "SourceGraphic"
        << "SourceAlpha"
        << "BackgroundImage"
        << "BackgroundAlpha"
        << "FillPaint"
        << "StrokePaint";

    QSet<QString> requiredInputs;
    if (isEmpty())
        return requiredInputs;

    if (d->filterEffects.first()->inputs().contains(""))
        requiredInputs.insert("SourceGraphic");

    foreach(KoFilterEffect *effect, d->filterEffects) {
        foreach(const QString &input, effect->inputs()) {
            if (stdInputs.contains(input))
                requiredInputs.insert(input);
        }
    }

    return requiredInputs;
}
