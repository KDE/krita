/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_OFFSET_KEEPER_H
#define __KIS_OFFSET_KEEPER_H


#include <kis_debug.h>
#include <QIODevice>

/**
 * Restore the offset of the io device on exit from the current
 * namespace
 */

class KisOffsetKeeper
{
public:

    KisOffsetKeeper(QIODevice *device)
        : m_device(device)
    {
        m_expectedPos = m_device->pos();
    }

    ~KisOffsetKeeper() {
        if (m_device->pos() != m_expectedPos) {
            m_device->seek(m_expectedPos);
        }
    }

private:
    QIODevice *m_device;
    qint64 m_expectedPos;
};

#endif /* __KIS_OFFSET_KEEPER_H */
