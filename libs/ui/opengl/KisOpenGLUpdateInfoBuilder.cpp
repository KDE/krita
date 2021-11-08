/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisOpenGLUpdateInfoBuilder.h"

// TODO: conversion options into a separate file!
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
    QSize effectiveTextureSize;

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
    return buildUpdateInfo(rect, srcImage->projection(), srcImage->bounds(), srcImage->currentLevelOfDetail(), convertColorSpace);
}

KisOpenGLUpdateInfoSP KisOpenGLUpdateInfoBuilder::buildUpdateInfo(const QRect &rect, KisPaintDeviceSP projection, const QRect &bounds, int levelOfDetail, bool convertColorSpace)
{
    KisOpenGLUpdateInfoSP info = new KisOpenGLUpdateInfo();

    QRect updateRect = rect & bounds;
    if (updateRect.isEmpty()) return info;

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_d->pool, info);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_d->conversionOptions.m_destinationColorSpace, info);

    auto needCreateProofingTransform =
        [this] () {
            return !m_d->proofingTransform &&
                m_d->proofingConfig &&
                m_d->proofingConfig->conversionFlags.testFlag(KoColorConversionTransformation::SoftProofing);
        };

    // lazily create transform
    if (convertColorSpace && needCreateProofingTransform()) {

        QWriteLocker locker(&m_d->lock);
        if (needCreateProofingTransform()) {
            const KoColorSpace *proofingSpace = KoColorSpaceRegistry::instance()->colorSpace(m_d->proofingConfig->proofingModel,
                                                                                             m_d->proofingConfig->proofingDepth,
                                                                                             m_d->proofingConfig->proofingProfile);

            m_d->proofingTransform.reset(KisTextureTileUpdateInfo::generateProofingTransform(
                                             projection->colorSpace(),
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
     * intentionally stretched to get through the overlapping
     * stripes of neutrality and poke neighbouring tiles.
     * Thanks to the rect we get the coordinates of all the tiles
     * involved into update process
     */

    QRect artificialRect = kisGrowRect(updateRect, m_d->textureBorder);
    artificialRect &= bounds;

    int firstColumn = xToCol(artificialRect.left());
    int lastColumn = xToCol(artificialRect.right());
    int firstRow = yToRow(artificialRect.top());
    int lastRow = yToRow(artificialRect.bottom());

    QBitArray channelFlags; // empty by default

    if (!m_d->channelFlags.isEmpty() &&
        m_d->channelFlags.size() == projection->colorSpace()->channels().size()) {

        channelFlags = m_d->channelFlags;
    }

    qint32 numItems = (lastColumn - firstColumn + 1) * (lastRow - firstRow + 1);
    info->tileList.reserve(numItems);

    QRect alignedUpdateRect = updateRect;
    QRect alignedBounds = bounds;

    if (levelOfDetail) {
        alignedUpdateRect = KisLodTransform::alignedRect(alignedUpdateRect, levelOfDetail);
        alignedBounds = KisLodTransform::alignedRect(alignedBounds, levelOfDetail);
    }

    for (int col = firstColumn; col <= lastColumn; col++) {
        for (int row = firstRow; row <= lastRow; row++) {

            const QRect alignedTileTextureRect = calculatePhysicalTileRect(col, row, bounds, levelOfDetail);

            KisTextureTileUpdateInfoSP tileInfo(
                        new KisTextureTileUpdateInfo(col, row,
                                                     alignedTileTextureRect,
                                                     alignedUpdateRect,
                                                     alignedBounds,
                                                     levelOfDetail,
                                                     m_d->pool));
            // Don't update empty tiles
            if (tileInfo->valid()) {
                tileInfo->retrieveData(projection, channelFlags, m_d->onlyOneChannelSelected, m_d->selectedChannelIndex);

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
                dbgUI << "Trying to create an empty tileinfo record" << col << row << alignedTileTextureRect << updateRect << bounds;
            }
        }
    }

    info->assignDirtyImageRect(rect);
    info->assignLevelOfDetail(levelOfDetail);
    return info;
}

QRect KisOpenGLUpdateInfoBuilder::calculateEffectiveTileRect(int col, int row, const QRect &imageBounds) const
{
    return imageBounds &
            QRect(col * m_d->effectiveTextureSize.width(),
                  row * m_d->effectiveTextureSize.height(),
                  m_d->effectiveTextureSize.width(),
                  m_d->effectiveTextureSize.height());
}

QRect KisOpenGLUpdateInfoBuilder::calculatePhysicalTileRect(int col, int row, const QRect &imageBounds, int levelOfDetail) const
{
    const QRect tileRect = calculateEffectiveTileRect(col, row, imageBounds);
    const QRect tileTextureRect = kisGrowRect(tileRect, m_d->textureBorder);

    const QRect alignedTileTextureRect = levelOfDetail ?
                KisLodTransform::alignedRect(tileTextureRect, levelOfDetail) :
                tileTextureRect;

    return alignedTileTextureRect;
}


int KisOpenGLUpdateInfoBuilder::xToCol(int x) const
{
    return x / m_d->effectiveTextureSize.width();
}

int KisOpenGLUpdateInfoBuilder::yToRow(int y) const
{
    return y / m_d->effectiveTextureSize.height();
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
    // the proofing transform becomes invalid when the target colorspace changes
    m_d->proofingTransform.reset();
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

    m_d->effectiveTextureSize = size;
}

void KisOpenGLUpdateInfoBuilder::setTextureInfoPool(KisTextureTileInfoPoolSP pool)
{
    QWriteLocker lock(&m_d->lock);

    m_d->pool = pool;
}

KisTextureTileInfoPoolSP KisOpenGLUpdateInfoBuilder::textureInfoPool() const
{
    QReadLocker lock(&m_d->lock);
    return m_d->pool;
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
