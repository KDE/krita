/*
 *  This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
        , m_randomAccessor(device->createRandomConstAccessorNG())
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
    
    int sumOfWeights = 0;

    weights[0] = qRound((1.0 - hsub) * (1.0 - vsub) * 255);
    sumOfWeights += weights[0];
    m_randomAccessor->moveTo(x, y);
    pixels[0] = m_randomAccessor->oldRawData();
    weights[1] = qRound((1.0 - vsub) * hsub * 255);
    sumOfWeights += weights[1];
    m_randomAccessor->moveTo(x + 1, y);
    pixels[1] = m_randomAccessor->oldRawData();
    weights[2] = qRound(vsub * (1.0 - hsub) * 255);
    sumOfWeights += weights[2];
    m_randomAccessor->moveTo(x, y + 1);
    pixels[2] = m_randomAccessor->oldRawData();
    weights[3] = qRound(hsub * vsub * 255);
    sumOfWeights += weights[3];
    m_randomAccessor->moveTo(x + 1, y + 1);
    pixels[3] = m_randomAccessor->oldRawData();

    m_device->colorSpace()->mixColorsOp()->mixColors(pixels, weights, 4, dst, sumOfWeights);
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
    
    int sumOfWeights = 0;

    weights[0] = qRound((1.0 - hsub) * (1.0 - vsub) * 255);
    sumOfWeights += weights[0];
    m_randomAccessor->moveTo(x, y);
    pixels[0] = m_randomAccessor->rawDataConst();
    weights[1] = qRound((1.0 - vsub) * hsub * 255);
    sumOfWeights += weights[1];
    m_randomAccessor->moveTo(x + 1, y);
    pixels[1] = m_randomAccessor->rawDataConst();
    weights[2] = qRound(vsub * (1.0 - hsub) * 255);
    sumOfWeights += weights[2];
    m_randomAccessor->moveTo(x, y + 1);
    pixels[2] = m_randomAccessor->rawDataConst();
    weights[3] = qRound(hsub * vsub * 255);
    sumOfWeights += weights[3];
    m_randomAccessor->moveTo(x + 1, y + 1);
    pixels[3] = m_randomAccessor->rawDataConst();
    m_device->colorSpace()->mixColorsOp()->mixColors(pixels, weights, 4, dst, sumOfWeights);
}
