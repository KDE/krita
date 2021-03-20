/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "KoFilterEffect.h"
#include "KoXmlWriter.h"

#include <QImage>
#include <QString>
#include <QRectF>

class Q_DECL_HIDDEN KoFilterEffect::Private
{
public:
    Private()
        : filterRect(0, 0, 1, 1)
        , requiredInputCount(1), maximalInputCount(1)
    {
        // add the default input
        inputs.append(QString());
    }

    QString id;
    QString name;
    QRectF filterRect;
    QList<QString> inputs;
    QString output;
    int requiredInputCount;
    int maximalInputCount;
};

KoFilterEffect::KoFilterEffect(const QString &id, const QString &name)
    : d(new Private)
{
    d->id = id;
    d->name = name;
}

KoFilterEffect::~KoFilterEffect()
{
    delete d;
}

QString KoFilterEffect::name() const
{
    return d->name;
}

QString KoFilterEffect::id() const
{
    return d->id;
}

void KoFilterEffect::setFilterRect(const QRectF &filterRect)
{
    d->filterRect = filterRect;
}

QRectF KoFilterEffect::filterRect() const
{
    return d->filterRect;
}

QRectF KoFilterEffect::filterRectForBoundingRect(const QRectF &boundingRect) const
{
    qreal x = boundingRect.x() + d->filterRect.x() * boundingRect.width();
    qreal y = boundingRect.y() + d->filterRect.y() * boundingRect.height();
    qreal w = d->filterRect.width() * boundingRect.width();
    qreal h = d->filterRect.height() * boundingRect.height();
    return QRectF(x, y, w, h);
}

QList<QString> KoFilterEffect::inputs() const
{
    return d->inputs;
}

void KoFilterEffect::addInput(const QString &input)
{
    if (d->inputs.count() < d->maximalInputCount)
        d->inputs.append(input);
}

void KoFilterEffect::insertInput(int index, const QString &input)
{
    if (d->inputs.count() < d->maximalInputCount)
        d->inputs.insert(index, input);
}

void KoFilterEffect::setInput(int index, const QString &input)
{
    if (index < d->inputs.count())
        d->inputs[index] = input;
}

void KoFilterEffect::removeInput(int index)
{
    if (d->inputs.count() > d->requiredInputCount)
        d->inputs.removeAt(index);
}

void KoFilterEffect::setOutput(const QString &output)
{
    d->output = output;
}

QString KoFilterEffect::output() const
{
    return d->output;
}

int KoFilterEffect::requiredInputCount() const
{
    return d->requiredInputCount;
}

int KoFilterEffect::maximalInputCount() const
{
    return qMax(d->maximalInputCount, d->requiredInputCount);
}

QImage KoFilterEffect::processImages(const QList<QImage> &images, const KoFilterEffectRenderContext &/*context*/) const
{
    Q_ASSERT(images.count());
    return images.first();
}

void KoFilterEffect::setRequiredInputCount(int count)
{
    d->requiredInputCount = qMax(0, count);
    for (int i = d->inputs.count(); i < d->requiredInputCount; ++i)
        d->inputs.append(QString());
}

void KoFilterEffect::setMaximalInputCount(int count)
{
    d->maximalInputCount = qMax(0,count);
    if (d->inputs.count() > maximalInputCount()) {
        int removeCount = d->inputs.count()-maximalInputCount();
        for (int i = 0; i < removeCount; ++i)
            d->inputs.pop_back();
    }
}

void KoFilterEffect::saveCommonAttributes(KoXmlWriter &writer)
{
    writer.addAttribute("result", output());
    if (requiredInputCount() == 1 && maximalInputCount() == 1 && d->inputs.count() == 1) {
        writer.addAttribute("in", d->inputs[0]);
    }
    writer.addAttribute("x", d->filterRect.x());
    writer.addAttribute("y", d->filterRect.y());
    writer.addAttribute("width", d->filterRect.width());
    writer.addAttribute("height", d->filterRect.height());
}
