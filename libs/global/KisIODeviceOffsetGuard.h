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


/**
 * A simple RAII-based class for restoring a position of QIODevice
 * when the code exits the scope.
 *
 * It is used to do multiple passes over a QIODevice. Just wrap each
 * pass in a namespace scope and add a guard. Please note that the
 * device must be !isSequential().
 *
 * @see KisOffsetOnExitVerifier
 */
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
