/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
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
#include "KisOpenGLUpdateInfoBuilder.h"

#include "kis_update_info.h"
#include "opengl/kis_texture_tile_info_pool.h"

#include "KisProofingConfiguration.h"

#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>


struct KRITAUI_NO_EXPORT KisOpenGLUpdateInfoBuilder::Private
{
    ConversionOptions conversionOptions;

    QBitArray channelFlags;
    bool onlyOneChannelSelected = false;
    int selectedChannelIndex = -1;

    int textureBorder = 0;
    QSize textureSize;

    KisProofingConfigurationSP proofingConfig;
    QScopedPointer<KoColorConversionTransformation> proofingTransform;

    KisTextureTileInfoPoolSP pool;
    QReadWriteLock lock;
};


KisOpenGLUpdateInfoBuilder::KisOpenGLUpdateInfoBuilder()
    : m_d(new Private)
{
}

KisOpenGLUpdateInfoBuilder::~KisOpenGLUpdateInfoBuilder()
{
}

KisOpenGLUpdateInfoSP KisOpenGLUpdateInfoBuilder::buildUpdateInfo(const QRect &rect, KisImageSP srcImage, bool convertColorSpace)
{
    KisOpenGLUpdateInfoSP info = new KisOpenGLUpdateInfo();

    QRect updateRect = rect & srcImage->bounds();
    if (updateRect.isEmpty()) return info;

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_d->pool, info);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_d->conversionOptions.m_destinationColorSpace, info);

    // lazily create transform
    if (convertColorSpace &&
        !m_d->proofingTransform && m_d->proofingConfig->conversionFlags.testFlag(KoColorConversionTransformation::SoftProofing)) {

        QWriteLocker locker(&m_d->lock);
        if (!m_d->proofingTransform && m_d->proofingConfig->conversionFlags.testFlag(KoColorConversionTransformation::SoftProofing)) {
            const KoColorSpace *proofingSpace = KoColorSpaceRegistry::instance()->colorSpace(m_d->proofingConfig->proofingModel,
                                                                                             m_d->proofingConfig->proofingDepth,
                                                                                             m_d->proofingConfig->proofingProfile);

            m_d->proofingTransform.reset(KisTextureTileUpdateInfo::generateProofingTransform(
                                             srcImage->projection()->colorSpace(),
                                             m_d->conversionOptions.m_destinationColorSpace,
                                             proofingSpace,
                                             m_d->conversionOptions.m_renderingIntent,
                                             m_d->proofingConfig->intent,
                                             m_d->proofingConfig->conversionFlags,
                                             m_d->proofingConfig->warningColor,
                                             m_d->proofingConfig->adaptationState));
        }
    }

    QReadLocker locker(&m_d->lock);

    /**
     * Why the rect is artificial? That's easy!
     * It does not represent any real piece of the image. It is
     * intentionally stretched to get through the overlappping
     * stripes of neutrality and poke neighbouring tiles.
     * Thanks to the rect we get the coordinates of all the tiles
     * involved into update process
     */

    QRect artificialRect = kisGrowRect(updateRect, m_d->textureBorder);
    artificialRect &= srcImage->bounds();

    int firstColumn = xToCol(artificialRect.left());
    int lastColumn = xToCol(artificialRect.right());
    int firstRow = yToRow(artificialRect.top());
    int lastRow = yToRow(artificialRect.bottom());

    QBitArray channelFlags; // empty by default

    if (!m_d->channelFlags.isEmpty() &&
        m_d->channelFlags.size() == srcImage->projection()->colorSpace()->channels().size()) {

        channelFlags = m_d->channelFlags;
    }

    qint32 numItems = (lastColumn - firstColumn + 1) * (lastRow - firstRow + 1);
    info->tileList.reserve(numItems);

    const QRect bounds = srcImage->bounds();
    const int levelOfDetail = srcImage->currentLevelOfDetail();

    QRect alignedUpdateRect = updateRect;
    QRect alignedBounds = bounds;

    if (levelOfDetail) {
        alignedUpdateRect = KisLodTransform::alignedRect(alignedUpdateRect, levelOfDetail);
        alignedBounds = KisLodTransform::alignedRect(alignedBounds, levelOfDetail);
    }

    for (int col = firstColumn; col <= lastColumn; col++) {
        for (int row = firstRow; row <= lastRow; row++) {

            const QRect tileRect = calculateTileRect(col, row, srcImage);
            const QRect tileTextureRect = kisGrowRect(tileRect, m_d->textureBorder);

            QRect alignedTileTextureRect = levelOfDetail ?
                        KisLodTransform::alignedRect(tileTextureRect, levelOfDetail) :
                        tileTextureRect;

            KisTextureTileUpdateInfoSP tileInfo(
                        new KisTextureTileUpdateInfo(col, row,
                                                     alignedTileTextureRect,
                                                     alignedUpdateRect,
                                                     alignedBounds,
                                                     levelOfDetail,
                                                     m_d->pool));
            // Don't update empty tiles
            if (tileInfo->valid()) {
                tileInfo->retrieveData(srcImage->projection(), channelFlags, m_d->onlyOneChannelSelected, m_d->selectedChannelIndex);

                if (convertColorSpace) {
                    if (m_d->proofingTransform) {
                        tileInfo->proofTo(m_d->conversionOptions.m_destinationColorSpace, m_d->proofingConfig->conversionFlags, m_d->proofingTransform.data());
                    } else {
                        tileInfo->convertTo(m_d->conversionOptions.m_destinationColorSpace, m_d->conversionOptions.m_renderingIntent, m_d->conversionOptions.m_conversionFlags);
                    }
                }

                info->tileList.append(tileInfo);
            }
            else {
                dbgUI << "Trying to create an empty tileinfo record" << col << row << tileTextureRect << updateRect << srcImage->bounds();
            }
        }
    }

    info->assignDirtyImageRect(rect);
    info->assignLevelOfDetail(levelOfDetail);
    return info;
}

QRect KisOpenGLUpdateInfoBuilder::calculateTileRect(int col, int row, KisImageSP image) const
{
    return image->bounds() &
            QRect(col * m_d->textureSize.width(),
                  row * m_d->textureSize.height(),
                  m_d->textureSize.width(),
                  m_d->textureSize.height());
}

int KisOpenGLUpdateInfoBuilder::xToCol(int x) const
{
    return x / m_d->textureSize.width();
}

int KisOpenGLUpdateInfoBuilder::yToRow(int y) const
{
    return y / m_d->textureSize.height();
}

const KoColorSpace *KisOpenGLUpdateInfoBuilder::destinationColorSpace() const
{
    QReadLocker lock(&m_d->lock);

    return m_d->conversionOptions.m_destinationColorSpace;
}

void KisOpenGLUpdateInfoBuilder::setConversionOptions(const ConversionOptions &options)
{
    QWriteLocker lock(&m_d->lock);

    m_d->conversionOptions = options;
}

void KisOpenGLUpdateInfoBuilder::setChannelFlags(const QBitArray &channelFrags, bool onlyOneChannelSelected, int selectedChannelIndex)
{
    QWriteLocker lock(&m_d->lock);

    m_d->channelFlags = channelFrags;
    m_d->onlyOneChannelSelected = onlyOneChannelSelected;
    m_d->selectedChannelIndex = selectedChannelIndex;
}

void KisOpenGLUpdateInfoBuilder::setTextureBorder(int value)
{
    QWriteLocker lock(&m_d->lock);

    m_d->textureBorder = value;
}

void KisOpenGLUpdateInfoBuilder::setEffectiveTextureSize(const QSize &size)
{
    QWriteLocker lock(&m_d->lock);

    m_d->textureSize = size;
}

void KisOpenGLUpdateInfoBuilder::setTextureInfoPool(KisTextureTileInfoPoolSP pool)
{
    QWriteLocker lock(&m_d->lock);

    m_d->pool = pool;
}

void KisOpenGLUpdateInfoBuilder::setProofingConfig(KisProofingConfigurationSP config)
{
    QWriteLocker lock(&m_d->lock);

    m_d->proofingConfig = config;
    m_d->proofingTransform.reset();
}

KisProofingConfigurationSP KisOpenGLUpdateInfoBuilder::proofingConfig() const
{
    QReadLocker lock(&m_d->lock);

    return m_d->proofingConfig;
}
