/*
 *  Copyright (c) 2010, Dmitry Kazakov <dimula73@gmail.com>
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
#ifndef KIS_TEXTURE_TILE_UPDATE_INFO_H_
#define KIS_TEXTURE_TILE_UPDATE_INFO_H_

#include <QMessageBox>
#include <QThreadStorage>
#include <QScopedArrayPointer>

#include <KoColorSpace.h>
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_config.h"
#include <KoColorConversionTransformation.h>
#include <KoChannelInfo.h>
#include <kis_lod_transform.h>
#include "kis_texture_tile_info_pool.h"


class KisTextureTileUpdateInfo;
typedef QSharedPointer<KisTextureTileUpdateInfo> KisTextureTileUpdateInfoSP;
typedef QVector<KisTextureTileUpdateInfoSP> KisTextureTileUpdateInfoSPList;

/**
 * A buffer object for temporary data needed during the update process.
 *
 * - the buffer is allocated from the common pool to avoid memory
 *   fragmentation
 *
 * - the buffer's lifetime defines the lifetime of the allocated chunk
 *   of memory, so you don't have to thing about free'ing the memory
 */

class DataBuffer
{
public:
    DataBuffer(KisTextureTileInfoPoolSP pool)
        : m_data(0),
          m_pixelSize(0),
          m_pool(pool)
    {
    }

    DataBuffer(int pixelSize, KisTextureTileInfoPoolSP pool)
        : m_data(0),
          m_pixelSize(0),
          m_pool(pool)
    {
        allocate(pixelSize);
    }

    DataBuffer(DataBuffer &&rhs)
        : m_data(rhs.m_data),
          m_pixelSize(rhs.m_pixelSize),
          m_pool(rhs.m_pool)
    {
        rhs.m_data = 0;
    }

    DataBuffer& operator=(DataBuffer &&rhs) {
        swap(rhs);
        return *this;
    }

    ~DataBuffer() {
        if (m_data) {
            m_pool->free(m_data, m_pixelSize);
        }
    }

    void allocate(int pixelSize) {
        Q_ASSERT(!m_data);

        m_pixelSize = pixelSize;
        m_data = m_pool->malloc(m_pixelSize);
    }

    inline quint8* data() const {
        return m_data;
    }

    void swap(DataBuffer &other) {
        std::swap(other.m_pixelSize, m_pixelSize);
        std::swap(other.m_data, m_data);
        std::swap(other.m_pool, m_pool);
    }

    int size() const {
        return m_data ? m_pool->chunkSize(m_pixelSize) : 0;
    }

    KisTextureTileInfoPoolSP pool() const {
        return m_pool;
    }

    int pixelSize() const {
        return m_pixelSize;
    }

private:
    Q_DISABLE_COPY(DataBuffer)

    quint8 *m_data;
    int m_pixelSize;
    KisTextureTileInfoPoolSP m_pool;
};

class KisTextureTileUpdateInfo
{
public:
    KisTextureTileUpdateInfo(KisTextureTileInfoPoolSP pool)
        : m_patchPixels(pool),
          m_pool(pool)
    {
    }

    KisTextureTileUpdateInfo(qint32 col, qint32 row,
                             const QRect &tileRect, const QRect &updateRect, const QRect &currentImageRect,
                             int levelOfDetail,
                             KisTextureTileInfoPoolSP pool)
        : m_patchPixels(pool),
          m_pool(pool)
    {
        m_tileCol = col;
        m_tileRow = row;
        m_tileRect = tileRect;
        m_originalTileRect = m_tileRect;
        m_patchRect = m_tileRect & updateRect;
        m_originalPatchRect = m_patchRect;
        m_currentImageRect = currentImageRect;

        m_patchLevelOfDetail = levelOfDetail;

        if (m_patchLevelOfDetail) {
            // TODO: check if isBottommost() works correctly when m_originalPatchRect gets aligned
            //       and m_currentImageRect has non-aligned size
            m_originalPatchRect = KisLodTransform::alignedRect(m_originalPatchRect, m_patchLevelOfDetail);
            m_patchRect = KisLodTransform::scaledRect(m_originalPatchRect, m_patchLevelOfDetail);
            m_tileRect = KisLodTransform::scaledRect(m_originalTileRect, m_patchLevelOfDetail);
        }
    }

    ~KisTextureTileUpdateInfo() {
    }

    void retrieveData(KisPaintDeviceSP projectionDevice, const QBitArray &channelFlags, bool onlyOneChannelSelected, int selectedChannelIndex)
    {
        m_patchColorSpace = projectionDevice->colorSpace();
        m_patchPixels.allocate(m_patchColorSpace->pixelSize());

        projectionDevice->readBytes(m_patchPixels.data(),
                                       m_patchRect.x(), m_patchRect.y(),
                                       m_patchRect.width(), m_patchRect.height());

        // XXX: if the paint colorspace is rgb, we should do the channel swizzling in
        //      the display shader
        if (!channelFlags.isEmpty() && selectedChannelIndex >= 0 && selectedChannelIndex < m_patchColorSpace->channels().size()) {
            DataBuffer conversionCache(m_patchColorSpace->pixelSize(), m_pool);

            QList<KoChannelInfo*> channelInfo = m_patchColorSpace->channels();
            int channelSize = channelInfo[selectedChannelIndex]->size();
            int pixelSize = m_patchColorSpace->pixelSize();
            quint32 numPixels = m_patchRect.width() * m_patchRect.height();

            KisConfig cfg(true);

            if (onlyOneChannelSelected && !cfg.showSingleChannelAsColor()) {
                int selectedChannelPos = channelInfo[selectedChannelIndex]->pos();
                for (uint pixelIndex = 0; pixelIndex < numPixels; ++pixelIndex) {
                    for (uint channelIndex = 0; channelIndex < m_patchColorSpace->channelCount(); ++channelIndex) {

                        if (channelInfo[channelIndex]->channelType() == KoChannelInfo::COLOR) {
                            memcpy(conversionCache.data() + (pixelIndex * pixelSize) + (channelIndex * channelSize),
                                   m_patchPixels.data() + (pixelIndex * pixelSize) + selectedChannelPos,
                                   channelSize);
                        }
                        else if (channelInfo[channelIndex]->channelType() == KoChannelInfo::ALPHA) {
                            memcpy(conversionCache.data() + (pixelIndex * pixelSize) + (channelIndex * channelSize),
                                   m_patchPixels.data() + (pixelIndex * pixelSize) + (channelIndex * channelSize),
                                   channelSize);
                        }
                    }
                }
            }
            else {
                for (uint pixelIndex = 0; pixelIndex < numPixels; ++pixelIndex) {
                    for (uint channelIndex = 0; channelIndex < m_patchColorSpace->channelCount(); ++channelIndex) {
                        if (channelFlags.testBit(channelIndex)) {
                            memcpy(conversionCache.data() + (pixelIndex * pixelSize) + (channelIndex * channelSize),
                                   m_patchPixels.data() + (pixelIndex * pixelSize) + (channelIndex * channelSize),
                                   channelSize);
                        }
                        else {
                            memset(conversionCache.data() + (pixelIndex * pixelSize) + (channelIndex * channelSize), 0, channelSize);
                        }
                    }
                }

            }

            conversionCache.swap(m_patchPixels);
        }

    }

    void convertTo(const KoColorSpace* dstCS,
                   KoColorConversionTransformation::Intent renderingIntent,
                   KoColorConversionTransformation::ConversionFlags conversionFlags)
    {
        // we use two-stage check of the color space equivalence:
        // first check pointers, and if not, check the spaces themselves
        if ((dstCS == m_patchColorSpace || *dstCS == *m_patchColorSpace) &&
            conversionFlags == KoColorConversionTransformation::Empty) {

            return;
        }

        if (m_patchRect.isValid()) {
            const qint32 numPixels = m_patchRect.width() * m_patchRect.height();
            DataBuffer conversionCache(dstCS->pixelSize(), m_pool);

            m_patchColorSpace->convertPixelsTo(m_patchPixels.data(), conversionCache.data(), dstCS, numPixels, renderingIntent, conversionFlags);

            m_patchColorSpace = dstCS;
            conversionCache.swap(m_patchPixels);
        }
    }

    void proofTo(const KoColorSpace* dstCS,
                   KoColorConversionTransformation::ConversionFlags conversionFlags,
                   KoColorConversionTransformation *proofingTransform)
    {
        if (dstCS == m_patchColorSpace && conversionFlags == KoColorConversionTransformation::Empty) return;

        if (m_patchRect.isValid()) {
            const qint32 numPixels = m_patchRect.width() * m_patchRect.height();
            DataBuffer conversionCache(dstCS->pixelSize(), m_pool);

            m_patchColorSpace->proofPixelsTo(m_patchPixels.data(), conversionCache.data(), numPixels, proofingTransform);

            m_patchColorSpace = dstCS;
            conversionCache.swap(m_patchPixels);
        }
    }

    static KoColorConversionTransformation *generateProofingTransform(const KoColorSpace* srcCS,
                                                                      const KoColorSpace* dstCS, const KoColorSpace* proofingSpace,
                                                                      KoColorConversionTransformation::Intent renderingIntent,
                                                                      KoColorConversionTransformation::Intent proofingIntent,
                                                                      KoColorConversionTransformation::ConversionFlags conversionFlags,
                                                                      KoColor gamutWarning,
                                                                      double adaptationState)
    {
        return srcCS->createProofingTransform(dstCS, proofingSpace, renderingIntent, proofingIntent, conversionFlags, gamutWarning.data(), adaptationState);
    }

    inline quint8* data() const {
        return m_patchPixels.data();
    }

    inline int patchLevelOfDetail() const {
        return m_patchLevelOfDetail;
    }

    inline QPoint realPatchOffset() const {
        return QPoint(m_patchRect.x() - m_tileRect.x(),
                      m_patchRect.y() - m_tileRect.y());
    }

    inline QSize realPatchSize() const {
        return m_patchRect.size();
    }

    inline QRect realPatchRect() const {
        return m_patchRect;
    }

    inline QSize realTileSize() const {
        return m_tileRect.size();
    }

    inline bool isTopmost() const {
        return m_originalPatchRect.top() == m_currentImageRect.top();
    }

    inline bool isLeftmost() const {
        return m_originalPatchRect.left() == m_currentImageRect.left();
    }

    inline bool isRightmost() const {
        return m_originalPatchRect.right() == m_currentImageRect.right();
    }

    inline bool isBottommost() const {
        return m_originalPatchRect.bottom() == m_currentImageRect.bottom();
    }

    inline bool isEntireTileUpdated() const {
        return m_patchRect == m_tileRect;
    }

    inline qint32 tileCol() const {
        return m_tileCol;
    }

    inline qint32 tileRow() const {
        return m_tileRow;
    }

    inline int pixelSize() const {
        return m_patchColorSpace->pixelSize();
    }

    inline const KoColorSpace* patchColorSpace() const {
        return m_patchColorSpace;
    }

    inline quint32 patchPixelsLength() const {
        return m_patchPixels.size();
    }

    inline bool valid() const {
        return m_patchRect.isValid();
    }

    inline DataBuffer&& takePixelData() {
        return std::move(m_patchPixels);
    }

    inline void putPixelData(DataBuffer &&buffer, const KoColorSpace *colorSpace) {
        m_patchPixels = std::move(buffer);
        m_patchColorSpace = colorSpace;
    }

private:
    Q_DISABLE_COPY(KisTextureTileUpdateInfo)

private:
    qint32 m_tileCol;
    qint32 m_tileRow;
    QRect m_currentImageRect;
    QRect m_tileRect;
    QRect m_patchRect;
    const KoColorSpace* m_patchColorSpace;

    QRect m_realPatchRect;
    QRect m_realPatchOffset;
    QRect m_realTileSize;
    int m_patchLevelOfDetail;

    QRect m_originalPatchRect;
    QRect m_originalTileRect;

    DataBuffer m_patchPixels;
    KisTextureTileInfoPoolSP m_pool;
};


#endif /* KIS_TEXTURE_TILE_UPDATE_INFO_H_ */

