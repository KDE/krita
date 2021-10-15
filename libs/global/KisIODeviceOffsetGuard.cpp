/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisIODeviceOffsetGuard.h"

#include <QIODevice>
#include "kis_assert.h"

KisIODeviceOffsetGuard::KisIODeviceOffsetGuard(QIODevice *device)
    : m_device(device),
      m_originalPos(device->pos())
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(device->isOpen());
    KIS_SAFE_ASSERT_RECOVER_NOOP(!device->isSequential());
}

KisIODeviceOffsetGuard::~KisIODeviceOffsetGuard()
{
    reset();
}

void KisIODeviceOffsetGuard::reset()
{
    m_device->seek(m_originalPos);
}
