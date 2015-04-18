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

#ifndef __KIS_ASL_WRITER_UTILS_H
#define __KIS_ASL_WRITER_UTILS_H

#include <stdexcept>
#include <string>

#include <QIODevice>

#include "psd_utils.h"
#include "kis_debug.h"
#include "libkispsd_export.h"


namespace KisAslWriterUtils {

/**
 * Exception that is emitted when any write error appear.
 */
struct LIBKISPSD_EXPORT ASLWriteException : public std::runtime_error
{
    ASLWriteException(const QString &msg)
        : std::runtime_error(msg.toAscii().data())
    {
    }
};

}

#define SAFE_WRITE_EX(device, varname)                                  \
    if (!psdwrite(device, varname)) {                                   \
        QString msg = QString("Failed to write \'%1\' tag!").arg(#varname); \
        throw KisAslWriterUtils::ASLWriteException(msg);                \
    }


namespace KisAslWriterUtils {

LIBKISPSD_EXPORT void writeUnicodeString(const QString &value, QIODevice *device);
LIBKISPSD_EXPORT void writeVarString(const QString &value, QIODevice *device);
LIBKISPSD_EXPORT void writePascalString(const QString &value, QIODevice *device);
LIBKISPSD_EXPORT void writeFixedString(const QString &value, QIODevice *device);
LIBKISPSD_EXPORT QString getPatternUuidLazy(const KoPattern *pattern);

/**
 * Align the pointer \p pos by alignment. Grow the pointer
 * if needed.
 *
 * \return the lowest integer not smaller than \p pos that divides by
 *         alignment
 */
inline qint64 alignOffsetCeil(qint64 pos, qint64 alignment)
{
    qint64 mask = alignment - 1;
    return (pos + mask) & ~mask;
}

template <class OffsetType>
class OffsetStreamPusher
{
public:
    OffsetStreamPusher(QIODevice *device)
        : m_device(device)
    {
        m_sizeFieldPos = m_device->pos();

        const OffsetType fakeObjectSize = 0xdeadbeef;
        SAFE_WRITE_EX(m_device, fakeObjectSize);
    }

    ~OffsetStreamPusher() {
        const qint64 currentPos = m_device->pos();
        const qint64 writtenDataSize = currentPos - m_sizeFieldPos - sizeof(OffsetType);

        m_device->seek(m_sizeFieldPos);
        const OffsetType realObjectSize = writtenDataSize;
        SAFE_WRITE_EX(m_device, realObjectSize);
        m_device->seek(currentPos);
    }

private:
    qint64 m_sizeFieldPos;
    QIODevice *m_device;
};

}

#endif /* __KIS_ASL_WRITER_UTILS_H */
