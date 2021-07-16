/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ASL_WRITER_UTILS_H
#define __KIS_ASL_WRITER_UTILS_H

#include <stdexcept>
#include <string>

#include <QIODevice>

#include "kis_debug.h"
#include "kritapsdutils_export.h"
#include "psd_utils.h"

namespace KisAslWriterUtils
{
/**
 * Exception that is emitted when any write error appear.
 */
struct KRITAPSDUTILS_EXPORT ASLWriteException : public std::runtime_error {
    ASLWriteException(const QString &msg)
        : std::runtime_error(msg.toLatin1().data())
    {
    }
};

}

#define SAFE_WRITE_EX(device, varname)                                                                                                                         \
    if (!psdwrite(device, varname)) {                                                                                                                          \
        QString msg = QString("Failed to write \'%1\' tag!").arg(#varname);                                                                                    \
        throw KisAslWriterUtils::ASLWriteException(msg);                                                                                                       \
    }

namespace KisAslWriterUtils
{
KRITAPSDUTILS_EXPORT void writeRect(const QRect &rect, QIODevice &device);
KRITAPSDUTILS_EXPORT void writeUnicodeString(const QString &value, QIODevice &device);
KRITAPSDUTILS_EXPORT void writeVarString(const QString &value, QIODevice &device);
KRITAPSDUTILS_EXPORT void writePascalString(const QString &value, QIODevice &device);
KRITAPSDUTILS_EXPORT void writeFixedString(const QString &value, QIODevice &device);
KRITAPSDUTILS_EXPORT QString getPatternUuidLazy(const KoPatternSP pattern);

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

template<class OffsetType>
class OffsetStreamPusher
{
public:
    OffsetStreamPusher(QIODevice &device, qint64 alignOnExit = 0, qint64 externalSizeTagOffset = -1)
        : m_device(device)
        , m_alignOnExit(alignOnExit)
        , m_externalSizeTagOffset(externalSizeTagOffset)
    {
        m_chunkStartPos = m_device.pos();

        if (externalSizeTagOffset < 0) {
            const OffsetType fakeObjectSize = OffsetType(0xdeadbeef);
            SAFE_WRITE_EX(m_device, fakeObjectSize);
        }
    }

    ~OffsetStreamPusher()
    {
        try {
            if (m_alignOnExit) {
                qint64 currentPos = m_device.pos();
                const qint64 alignedPos = alignOffsetCeil(currentPos, m_alignOnExit);

                for (; currentPos < alignedPos; currentPos++) {
                    quint8 padding = 0;
                    SAFE_WRITE_EX(m_device, padding);
                }
            }

            const qint64 currentPos = m_device.pos();

            qint64 writtenDataSize = 0;
            qint64 sizeFiledOffset = 0;

            if (m_externalSizeTagOffset >= 0) {
                writtenDataSize = currentPos - m_chunkStartPos;
                sizeFiledOffset = m_externalSizeTagOffset;
            } else {
                writtenDataSize = currentPos - m_chunkStartPos - sizeof(OffsetType);
                sizeFiledOffset = m_chunkStartPos;
            }

            m_device.seek(sizeFiledOffset);
            const OffsetType realObjectSize = writtenDataSize;
            SAFE_WRITE_EX(m_device, realObjectSize);
            m_device.seek(currentPos);
        } catch (ASLWriteException &e) {
            warnKrita << PREPEND_METHOD(e.what());
        }
    }

private:
    qint64 m_chunkStartPos;
    QIODevice &m_device;
    qint64 m_alignOnExit;
    qint64 m_externalSizeTagOffset;
};

}

#endif /* __KIS_ASL_WRITER_UTILS_H */
