/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_transform_strategy_base.h"

#include <QImage>
#include <QPainterPath>
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

void KisTransformStrategyBase::activatePrimaryAction()
{
}

void KisTransformStrategyBase::deactivatePrimaryAction()
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
