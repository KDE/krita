/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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
