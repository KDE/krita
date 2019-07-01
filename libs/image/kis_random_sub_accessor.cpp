/*
 *  This file is part of the KDE project
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include "kis_random_sub_accessor.h"
#include <QtGlobal>

#include <KoColorSpace.h>
#include <KoMixColorsOp.h>
#include <QtMath>

#include "kis_paint_device.h"

KisRandomSubAccessor::KisRandomSubAccessor(KisPaintDeviceSP device)
        : m_device(device)
        , m_currentPoint(0, 0)
        , m_randomAccessor(device->createRandomConstAccessorNG(0, 0))
{
}


KisRandomSubAccessor::~KisRandomSubAccessor()
{
}


void KisRandomSubAccessor::sampledOldRawData(quint8* dst)
{
    const quint8* pixels[4];
    qint16 weights[4];
    int x = qFloor(m_currentPoint.x());
    int y = qFloor(m_currentPoint.y());

    double hsub = m_currentPoint.x() - x;
    if (hsub < 0.0) {
        hsub = 1.0 + hsub;
    }
    double vsub = m_currentPoint.y() - y;
    if (vsub < 0.0) {
        vsub = 1.0 + vsub;
    }

    weights[0] = qRound((1.0 - hsub) * (1.0 - vsub) * 255);
    m_randomAccessor->moveTo(x, y);
    pixels[0] = m_randomAccessor->oldRawData();
    weights[1] = qRound((1.0 - vsub) * hsub * 255);
    m_randomAccessor->moveTo(x + 1, y);
    pixels[1] = m_randomAccessor->oldRawData();
    weights[2] = qRound(vsub * (1.0 - hsub) * 255);
    m_randomAccessor->moveTo(x, y + 1);
    pixels[2] = m_randomAccessor->oldRawData();
    weights[3] = qRound(hsub * vsub * 255);
    m_randomAccessor->moveTo(x + 1, y + 1);
    pixels[3] = m_randomAccessor->oldRawData();

    m_device->colorSpace()->mixColorsOp()->mixColors(pixels, weights, 4, dst);
}


void KisRandomSubAccessor::sampledRawData(quint8* dst)
{
    const quint8* pixels[4];
    qint16 weights[4];
    int x = qFloor(m_currentPoint.x());
    int y = qFloor(m_currentPoint.y());

    double hsub = m_currentPoint.x() - x;
    if (hsub < 0.0) {
        hsub = 1.0 + hsub;
    }
    double vsub = m_currentPoint.y() - y;
    if (vsub < 0.0) {
        vsub = 1.0 + vsub;
    }

    weights[0] = qRound((1.0 - hsub) * (1.0 - vsub) * 255);
    m_randomAccessor->moveTo(x, y);
    pixels[0] = m_randomAccessor->rawDataConst();
    weights[1] = qRound((1.0 - vsub) * hsub * 255);
    m_randomAccessor->moveTo(x + 1, y);
    pixels[1] = m_randomAccessor->rawDataConst();
    weights[2] = qRound(vsub * (1.0 - hsub) * 255);
    m_randomAccessor->moveTo(x, y + 1);
    pixels[2] = m_randomAccessor->rawDataConst();
    weights[3] = qRound(hsub * vsub * 255);
    m_randomAccessor->moveTo(x + 1, y + 1);
    pixels[3] = m_randomAccessor->rawDataConst();
    m_device->colorSpace()->mixColorsOp()->mixColors(pixels, weights, 4, dst);
}
