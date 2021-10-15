/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISIODEVICEOFFSETGUARD_H
#define KISIODEVICEOFFSETGUARD_H

#include <kritaglobal_export.h>
#include <QtGlobal>

class QIODevice;

class KRITAGLOBAL_EXPORT KisIODeviceOffsetGuard
{
public:
    KisIODeviceOffsetGuard(QIODevice *device);
    ~KisIODeviceOffsetGuard();

    void reset();

private:
    QIODevice *m_device = 0;
    qint64 m_originalPos = 0;
};

#endif // KISIODEVICEOFFSETGUARD_H
