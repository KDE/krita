/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ASL_WRITER_UTILS_H
#define __KIS_ASL_WRITER_UTILS_H

#include "kritapsdutils_export.h"

#include <stdexcept>
#include <string>

#include <QIODevice>
#include <QUuid>

#include <kis_debug.h>
#include <resources/KoPattern.h>

#include "psd.h"
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

#define SAFE_WRITE_EX(byteOrder, device, varname)                                                                                                              \
    if (!psdwrite<byteOrder>(device, varname)) {                                                                                                               \
        QString msg = QString("Failed to write \'%1\' tag!").arg(#varname);                                                                                    \
        throw KisAslWriterUtils::ASLWriteException(msg);                                                                                                       \
    }

namespace KisAslWriterUtils
{
// XXX: rect uses variable-sized type, is this correct?
template<psd_byte_order byteOrder>
inline void writeRect(const QRect &rect, QIODevice &device)
{
    {
        const qint32 rectY0 = static_cast<qint32>(rect.y());
        SAFE_WRITE_EX(byteOrder, device, rectY0);
    }
    {
        const qint32 rectX0 = static_cast<qint32>(rect.x());
        SAFE_WRITE_EX(byteOrder, device, rectX0);
    }
    {
        const qint32 rectY1 = static_cast<qint32>(rect.y() + rect.height());
        SAFE_WRITE_EX(byteOrder, device, rectY1);
    }
    {
        const qint32 rectX1 = static_cast<qint32>(rect.x() + rect.width());
        SAFE_WRITE_EX(byteOrder, device, rectX1);
    }
}

template<psd_byte_order byteOrder>
inline void writeUnicodeString(const QString &value, QIODevice &device)
{
    const quint32 len = static_cast<quint32>(value.length() + 1);
    SAFE_WRITE_EX(byteOrder, device, len);

    const quint16 *ptr = value.utf16();
    for (quint32 i = 0; i < len; i++) {
        SAFE_WRITE_EX(byteOrder, device, ptr[i]);
    }
}

template<psd_byte_order byteOrder>
inline void writeVarString(const QString &value, QIODevice &device)
{
    const quint32 lenTag = static_cast<quint32>(value.length() != 4 ? value.length() : 0);
    SAFE_WRITE_EX(byteOrder, device, lenTag);

    if (!device.write(value.toLatin1().data(), value.length())) {
        warnKrita << "WARNING: ASL: Failed to write ASL string" << ppVar(value);
        return;
    }
}

template<psd_byte_order byteOrder>
inline void writePascalString(const QString &value, QIODevice &device)
{
    KIS_ASSERT_RECOVER_RETURN(value.length() < 256);
    KIS_ASSERT_RECOVER_RETURN(value.length() >= 0);
    const quint8 lenTag = static_cast<quint8>(value.length());
    SAFE_WRITE_EX(byteOrder, device, lenTag);

    if (!device.write(value.toLatin1().data(), value.length())) {
        warnKrita << "WARNING: ASL: Failed to write ASL string" << ppVar(value);
        return;
    }
}

template<psd_byte_order byteOrder>
inline void writeFixedString(const QString &value, QIODevice &device)
{
    KIS_ASSERT_RECOVER_RETURN(value.length() == 4);

    QByteArray data = value.toLatin1();

    if (byteOrder == psd_byte_order::psdLittleEndian) {
        std::reverse(data.begin(), data.end());
    }

    if (!device.write(data.data(), value.length())) {
        warnKrita << "WARNING: ASL: Failed to write ASL string" << ppVar(value);
        return;
    }
}

// Write UUID fetched from the file name or generate
template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
inline QString getPatternUuidLazy(const KoPatternSP pattern)
{
    QUuid uuid;
    QString patternFileName = pattern->filename();

    if (patternFileName.endsWith(".pat", Qt::CaseInsensitive)) {
        QString strUuid = patternFileName.left(patternFileName.size() - 4);

        uuid = QUuid(strUuid);
    }

    if (uuid.isNull()) {
        warnKrita << "WARNING: Saved pattern doesn't have a UUID, generating...";
        warnKrita << ppVar(patternFileName) << ppVar(pattern->name());
        uuid = QUuid::createUuid();
    }

    return uuid.toString().mid(1, 36);
}

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

template<class OffsetType, psd_byte_order byteOrder>
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
            SAFE_WRITE_EX(byteOrder, m_device, fakeObjectSize);
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
                    SAFE_WRITE_EX(byteOrder, m_device, padding);
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
            SAFE_WRITE_EX(byteOrder, m_device, realObjectSize);
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
