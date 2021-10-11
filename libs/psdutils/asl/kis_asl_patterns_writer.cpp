/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_asl_patterns_writer.h"

#include <functional>

#include <compression.h>
#include <kis_debug.h>
#include <resources/KoPattern.h>

#include "kis_asl_callback_object_catcher.h"
#include "kis_asl_writer_utils.h"
#include "kis_asl_xml_parser.h"

KisAslPatternsWriter::KisAslPatternsWriter(const QDomDocument &doc, QIODevice &device, psd_byte_order byteOrder)
    : m_doc(doc)
    , m_device(device)
    , m_numPatternsWritten(0)
    , m_byteOrder(byteOrder)
{
}

void KisAslPatternsWriter::writePatterns()
{
    KisAslCallbackObjectCatcher c;
    c.subscribePattern("/Patterns/KisPattern", std::bind(&KisAslPatternsWriter::addPattern, this, std::placeholders::_1));
    c.subscribePattern("/patterns/KisPattern", std::bind(&KisAslPatternsWriter::addPattern, this, std::placeholders::_1));

    KisAslXmlParser parser;
    parser.parseXML(m_doc, c);
}

void sliceQImage(const QImage &image, QVector<QVector<QByteArray>> *dstPlanes, bool *isCompressed)
{
    KIS_ASSERT_RECOVER_NOOP(image.format() == QImage::Format_ARGB32);

    QVector<QVector<QByteArray>> uncompressedRows;
    QVector<QVector<QByteArray>> compressedRows;

    uncompressedRows.resize(3);
    compressedRows.resize(3);

    int compressedSize = 0;

    for (int i = 0; i < 3; i++) {
        const int srcRowOffset = 2 - i;
        const int srcStep = 4;
        const int dstStep = 1;

        for (int row = 0; row < image.height(); row++) {
            uncompressedRows[i].append(QByteArray(image.width(), '\0'));
            quint8 *dstPtr = (quint8 *)uncompressedRows[i].last().data();

            const quint8 *srcPtr = image.constScanLine(row) + srcRowOffset;

            for (int col = 0; col < image.width(); col++) {
                *dstPtr = *srcPtr;

                srcPtr += srcStep;
                dstPtr += dstStep;
            }

            compressedRows[i].append(Compression::compress(uncompressedRows[i].last(), psd_compression_type::RLE));
            if (compressedRows[i].last().isEmpty()) {
                throw KisAslWriterUtils::ASLWriteException("Failed to compress pattern plane");
            }

            compressedSize += compressedRows[i].last().size() + 2; // two bytes for offset tag
        }
    }

    if (compressedSize < image.width() * image.height() * 3) {
        *dstPlanes = compressedRows;
        *isCompressed = true;
    } else {
        *dstPlanes = uncompressedRows;
        *isCompressed = false;
    }
}

void KisAslPatternsWriter::addPattern(const KoPatternSP pattern)
{
    KoPatternSP effectivePattern = pattern;

    if (effectivePattern->hasAlpha()) {
        effectivePattern = pattern->cloneWithoutAlpha();
    }

    switch (m_byteOrder) {
    case psd_byte_order::psdLittleEndian:
        addPatternImpl<psd_byte_order::psdLittleEndian>(pattern);
        break;
    default:
        addPatternImpl(pattern);
        break;
    }
}

template<psd_byte_order byteOrder>
void KisAslPatternsWriter::addPatternImpl(const KoPatternSP pattern)
{
    {
        KisAslWriterUtils::OffsetStreamPusher<quint32, byteOrder> patternSizeField(m_device);

        {
            const quint32 patternVersion = 1;
            SAFE_WRITE_EX(byteOrder, m_device, patternVersion);
        }

        {
            const quint32 patternImageMode = 3;
            SAFE_WRITE_EX(byteOrder, m_device, patternImageMode);
        }

        {
            const quint16 patternHeight = static_cast<quint16>(pattern->height());
            SAFE_WRITE_EX(byteOrder, m_device, patternHeight);
        }

        {
            const quint16 patternWidth = static_cast<quint16>(pattern->width());
            SAFE_WRITE_EX(byteOrder, m_device, patternWidth);
        }

        KisAslWriterUtils::writeUnicodeString<byteOrder>(pattern->name(), m_device);
        KisAslWriterUtils::writePascalString<byteOrder>(KisAslWriterUtils::getPatternUuidLazy(pattern), m_device);

        // Write "Virtual Memory Array List"

        const QRect patternRect(0, 0, pattern->width(), pattern->height());

        {
            {
                const quint32 arrayVersion = 3;
                SAFE_WRITE_EX(byteOrder, m_device, arrayVersion);
            }

            KisAslWriterUtils::OffsetStreamPusher<quint32, byteOrder> arraySizeField(m_device);

            KisAslWriterUtils::writeRect<byteOrder>(patternRect, m_device);

            {
                // don't ask me why it is called this way...
                const quint32 numberOfChannels = 24;
                SAFE_WRITE_EX(byteOrder, m_device, numberOfChannels);
            }

            KIS_ASSERT_RECOVER_RETURN(patternRect.size() == pattern->pattern().size());

            QVector<QVector<QByteArray>> imagePlanes;
            bool isCompressed;
            sliceQImage(pattern->pattern(), &imagePlanes, &isCompressed);

            for (int i = 0; i < 3; i++) {
                {
                    const quint32 planeIsWritten = 1;
                    SAFE_WRITE_EX(byteOrder, m_device, planeIsWritten);
                }

                KisAslWriterUtils::OffsetStreamPusher<quint32, byteOrder> planeSizeField(m_device);

                {
                    const quint32 pixelDepth1 = 8;
                    SAFE_WRITE_EX(byteOrder, m_device, pixelDepth1);
                }

                KisAslWriterUtils::writeRect<byteOrder>(patternRect, m_device);

                {
                    // why twice? who knows...
                    const quint16 pixelDepth2 = 8;
                    SAFE_WRITE_EX(byteOrder, m_device, pixelDepth2);
                }

                {
                    // compress with RLE
                    const quint8 compressionMethod = isCompressed;
                    SAFE_WRITE_EX(byteOrder, m_device, compressionMethod);
                }

                KIS_ASSERT_RECOVER_RETURN(imagePlanes[i].size() == pattern->pattern().height());

                if (isCompressed) {
                    Q_FOREACH (const QByteArray &compressedRow, imagePlanes[i]) {
                        const quint16 compressionRowSize = static_cast<quint16>(compressedRow.size());
                        SAFE_WRITE_EX(byteOrder, m_device, compressionRowSize);
                    }
                }

                Q_FOREACH (const QByteArray &rowData, imagePlanes[i]) {
                    const qint64 bytesWritten = m_device.write(rowData);
                    if (bytesWritten != rowData.size()) {
                        throw KisAslWriterUtils::ASLWriteException("Failed to write a compressed pattern plane");
                    }
                }
            }
        }
    }

    const qint64 currentPos = m_device.pos();
    const qint64 alignedPos = KisAslWriterUtils::alignOffsetCeil(currentPos, 4);

    if (currentPos != alignedPos) {
        m_device.seek(alignedPos);
    }

    m_numPatternsWritten++;
}
