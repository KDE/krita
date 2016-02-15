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


struct KisTransformStrategyBase::Private
{
    QTransform thumbToImageTransform;
    QImage originalImage;
};


KisTransformStrategyBase::KisTransformStrategyBase()
    : m_d(new Private())
{
}

KisTransformStrategyBase::~KisTransformStrategyBase()
{
}

QPainterPath KisTransformStrategyBase::getCursorOutline() const
{
    return QPainterPath();
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

void KisTransformStrategyBase::activateAlternateAction(KisTool::AlternateAction action)
{
    Q_UNUSED(action);
}

void KisTransformStrategyBase::deactivateAlternateAction(KisTool::AlternateAction action)
{
    Q_UNUSED(action);
}

bool KisTransformStrategyBase::beginAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action)
{
    Q_UNUSED(event);
    Q_UNUSED(action);
    return false;
}

void KisTransformStrategyBase::continueAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action)
{
    Q_UNUSED(event);
    Q_UNUSED(action);
}

bool KisTransformStrategyBase::endAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action)
{
    Q_UNUSED(event);
    Q_UNUSED(action);
    return false;
}
