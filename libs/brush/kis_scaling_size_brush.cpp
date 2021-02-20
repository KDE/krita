/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_scaling_size_brush.h"

KisScalingSizeBrush::KisScalingSizeBrush()
    : KisBrush()
{
}

KisScalingSizeBrush::KisScalingSizeBrush(const QString &filename)
    : KisBrush(filename)
{
}

KisScalingSizeBrush::KisScalingSizeBrush(const KisScalingSizeBrush &rhs)
    : KisBrush(rhs)
{
    setName(rhs.name());
    setValid(rhs.valid());
}

qreal KisScalingSizeBrush::userEffectiveSize() const
{
    return this->width() * this->scale();
}

void KisScalingSizeBrush::setUserEffectiveSize(qreal value)
{
    this->setScale(value / this->width());
}


