/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tile_compressor_2.h"
#include "kis_lzf_compression.h"
#include <QIODevice>
#include "kis_paint_device_writer.h"
#define TILE_DATA_SIZE(pixelSize) ((pixelSize) * KisTileData::WIDTH * KisTileData::HEIGHT)

const QString KisTileCompressor2::m_compressionName = "LZF";


KisTileCompressor2::KisTileCompressor2()
{
    m_compression = new KisLzfCompression();
}

KisTileCompressor2::~KisTileCompressor2()
{
    delete m_compression;
}

bool KisTileCompressor2::writeTile(KisTileSP tile, KisPaintDeviceWriter &store)
{
    const qint32 tileDataSize = TILE_DATA_SIZE(tile->pixelSize());
    prepareStreamingBuffer(tileDataSize);

    qint32 bytesWritten;

    tile->lockForRead();
    compressTileData(tile->tileData(), (quint8*)m_streamingBuffer.data(),
                     m_streamingBuffer.size(), bytesWritten);
    tile->unlockForRead();

    QString header = getHeader(tile, bytesWritten);
    bool retval = true;
    retval = store.write(header.toLatin1());
    if (!retval) {
        warnFile << "Failed to write the tile header";
    }
    retval = store.write(m_streamingBuffer.data(), bytesWritten);
    if (!retval) {
        warnFile << "Failed to write the tile data";
    }
    return retval;
}

bool KisTileCompressor2::readTile(QIODevice *stream, KisTiledDataManager *dm)
{
    const qint32 tileDataSize = TILE_DATA_SIZE(pixelSize(dm));
    prepareStreamingBuffer(tileDataSize);

    QByteArray header = stream->readLine(maxHeaderLength());

    QList<QByteArray> headerItems = header.trimmed().split(',');
    if (headerItems.size() == 4) {
        qint32 x = headerItems.takeFirst().toInt();
        qint32 y = headerItems.takeFirst().toInt();
        QString compressionName = headerItems.takeFirst();
        qint32 dataSize = headerItems.takeFirst().toInt();

        Q_ASSERT(headerItems.isEmpty());
        Q_ASSERT(compressionName == m_compressionName);

        qint32 row = yToRow(dm, y);
        qint32 col = xToCol(dm, x);

        KisTileSP tile = dm->getTile(col, row, true);

        stream->read(m_streamingBuffer.data(), dataSize);

        tile->lockForWrite();
        bool res = decompressTileData((quint8*)m_streamingBuffer.data(), dataSize, tile->tileData());
        tile->unlockForWrite();
        return res;
    }
    return false;
}

void KisTileCompressor2::prepareStreamingBuffer(qint32 tileDataSize)
{
    /**
     * TODO: delete this buffer!
     * It is better to use one of other two buffers to store streams
     */
    m_streamingBuffer.resize(tileDataSize + 1);
}

void KisTileCompressor2::prepareWorkBuffers(qint32 tileDataSize)
{
    const qint32 bufferSize = m_compression->outputBufferSize(tileDataSize);

    if (m_linearizationBuffer.size() < tileDataSize) {
        m_linearizationBuffer.resize(tileDataSize);
    }

    if (m_compressionBuffer.size() < bufferSize) {
        m_compressionBuffer.resize(bufferSize);
    }
}

void KisTileCompressor2::compressTileData(KisTileData *tileData,
                                          quint8 *buffer,
                                          qint32 bufferSize,
                                          qint32 &bytesWritten)
{
    const qint32 pixelSize = tileData->pixelSize();
    const qint32 tileDataSize = TILE_DATA_SIZE(pixelSize);
    qint32 compressedBytes;

    Q_UNUSED(bufferSize);
    Q_ASSERT(bufferSize >= tileDataSize + 1);

    prepareWorkBuffers(tileDataSize);

    KisAbstractCompression::linearizeColors(tileData->data(), (quint8*)m_linearizationBuffer.data(),
                                            tileDataSize, pixelSize);

    compressedBytes = m_compression->compress((quint8*)m_linearizationBuffer.data(), tileDataSize,
                                              (quint8*)m_compressionBuffer.data(), m_compressionBuffer.size());

    if(compressedBytes < tileDataSize) {
        buffer[0] = COMPRESSED_DATA_FLAG;
        memcpy(buffer + 1, m_compressionBuffer.data(), compressedBytes);
        bytesWritten = compressedBytes + 1;
    }
    else {
        buffer[0] = RAW_DATA_FLAG;
        memcpy(buffer + 1, tileData->data(), tileDataSize);
        bytesWritten = tileDataSize + 1;
    }
}

bool KisTileCompressor2::decompressTileData(quint8 *buffer,
                                            qint32 bufferSize,
                                            KisTileData *tileData)
{
    const qint32 pixelSize = tileData->pixelSize();
    const qint32 tileDataSize = TILE_DATA_SIZE(pixelSize);

    if(buffer[0] == COMPRESSED_DATA_FLAG) {
        prepareWorkBuffers(tileDataSize);

        qint32 bytesWritten;
        bytesWritten = m_compression->decompress(buffer + 1, bufferSize - 1,
                                                 (quint8*)m_linearizationBuffer.data(), tileDataSize);
        if (bytesWritten == tileDataSize) {
            KisAbstractCompression::delinearizeColors((quint8*)m_linearizationBuffer.data(),
                                                      tileData->data(),
                                                      tileDataSize, pixelSize);
            return true;
        }
        return false;
    }
    else {
        memcpy(tileData->data(), buffer + 1, tileDataSize);
        return true;
    }
    return false;

}

qint32 KisTileCompressor2::tileDataBufferSize(KisTileData *tileData)
{
    return TILE_DATA_SIZE(tileData->pixelSize()) + 1;
}

inline qint32 KisTileCompressor2::maxHeaderLength()
{
    static const qint32 QINT32_LENGTH = 11;
    static const qint32 COMPRESSION_NAME_LENGTH = 5;
    static const qint32 SEPARATORS_LENGTH = 4;

    return 3 * QINT32_LENGTH + COMPRESSION_NAME_LENGTH + SEPARATORS_LENGTH;
}

inline QString KisTileCompressor2::getHeader(KisTileSP tile,
                                             qint32 compressedSize)
{
    qint32 x, y;
    qint32 width, height;
    tile->extent().getRect(&x, &y, &width, &height);

    return QString("%1,%2,%3,%4\n").arg(x).arg(y).arg(m_compressionName).arg(compressedSize);
}
