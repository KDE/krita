/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
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

#include "DualBrush.h"

#include "kis_global.h"

#include <KoColor.h>
#include <KoColorSpace.h>

#include <QVariant>
#include <QHash>

#include "kis_random_accessor_ng.h"
#include <cmath>
#include <ctime>


DualBrushBrush::DualBrushBrush(const DualBrushProperties* properties)
{
    m_properties = properties;
}


DualBrushBrush::~DualBrushBrush()
{
}

void DualBrushBrush::paint(KisPaintDeviceSP dev, qreal x, qreal y, const KoColor &color, qreal additionalScale)
{
}

