/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoPostscriptPaintDevice.h"

#include <limits.h>

KoPostscriptPaintDevice::KoPostscriptPaintDevice()
{
}

QPaintEngine *KoPostscriptPaintDevice::paintEngine() const
{
    return 0;
}

int KoPostscriptPaintDevice::metric(QPaintDevice::PaintDeviceMetric metric) const
{
    switch (metric) {
    case QPaintDevice::PdmWidth:
    case QPaintDevice::PdmHeight:
    case QPaintDevice::PdmWidthMM:
    case QPaintDevice::PdmHeightMM:
    case QPaintDevice::PdmNumColors:
        return INT_MAX;
    case QPaintDevice::PdmDepth:
        return 32;
    case QPaintDevice::PdmDpiX:
    case QPaintDevice::PdmDpiY:
    case QPaintDevice::PdmPhysicalDpiX:
    case QPaintDevice::PdmPhysicalDpiY:
    default:
        return 72;
    }
    return 0; // should never be hit
}
