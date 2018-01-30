/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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
