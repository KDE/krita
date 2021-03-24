/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisInterstrokeData.h"

#include <KoColorSpace.h>
#include <kis_paint_device.h>

KisInterstrokeData::KisInterstrokeData(KisPaintDeviceSP device)
    : m_linkedDeviceOffset(device->offset())
    , m_linkedColorSpace(device->colorSpace())
    , m_linkedPaintDevice(device)
{

}

KisInterstrokeData::~KisInterstrokeData()
{
}

bool KisInterstrokeData::isStillCompatible() const
{
    KIS_ASSERT_RECOVER_RETURN_VALUE(m_linkedPaintDevice, false);

    return m_linkedDeviceOffset == m_linkedPaintDevice->offset() &&
        *m_linkedColorSpace == *m_linkedPaintDevice->colorSpace();
}
