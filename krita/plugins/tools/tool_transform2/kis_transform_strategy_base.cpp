/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_transform_strategy_base.h"

#include <QImage>
#include <QTransform>
#include "KoPointerEvent.h"
#include "kis_coordinates_converter.h"


struct KisTransformStrategyBase::Private
{
    Private(const KisCoordinatesConverter *_converter)
        : converter(_converter) {}

    QTransform thumbToImageTransform;
    QImage originalImage;
    const KisCoordinatesConverter *converter;
};


KisTransformStrategyBase::KisTransformStrategyBase(const KisCoordinatesConverter *_converter)
    : m_d(new Private(_converter))
{
}

KisTransformStrategyBase::~KisTransformStrategyBase()
{
}

QImage KisTransformStrategyBase::originalImage() const
{
    return m_d->originalImage;
}

QTransform KisTransformStrategyBase::thumbToImageTransform() const
{
    return m_d->thumbToImageTransform;
}

void KisTransformStrategyBase::setThumbnailImage(const QImage &image, QTransform thumbToImageTransform)
{
    m_d->originalImage = image;
    m_d->thumbToImageTransform = thumbToImageTransform;
}

bool KisTransformStrategyBase::acceptsClicks() const
{
    return false;
}

bool KisTransformStrategyBase::beginPrimaryAction(KoPointerEvent *event)
{
    return beginPrimaryAction(m_d->converter->documentToImage(event->point));
}

void KisTransformStrategyBase::continuePrimaryAction(KoPointerEvent *event, bool specialModifierActve)
{
    continuePrimaryAction(m_d->converter->documentToImage(event->point), specialModifierActve);
}

bool KisTransformStrategyBase::endPrimaryAction(KoPointerEvent *event)
{
    Q_UNUSED(event);
    return endPrimaryAction();
}

bool KisTransformStrategyBase::beginPrimaryAction(const QPointF &pt)
{
    Q_UNUSED(pt);
    qFatal("Not implemented");
    return false;
}

void KisTransformStrategyBase::continuePrimaryAction(const QPointF &pt, bool specialModifierActve)
{
    Q_UNUSED(pt);
    Q_UNUSED(specialModifierActve);
    qFatal("Not implemented");
}

bool KisTransformStrategyBase::endPrimaryAction()
{
    qFatal("Not implemented");
    return false;
}
