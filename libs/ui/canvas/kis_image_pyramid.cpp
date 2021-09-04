/*
 *  SPDX-FileCopyrightText: 2009 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_image_pyramid.h"

#include <QBitArray>
#include <KoChannelInfo.h>
#include <KoCompositeOp.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpaceMaths.h>

#include "kis_display_filter.h"
#include "kis_painter.h"
#include "kis_iterator_ng.h"
#include "kis_datamanager.h"
#include "kis_config_notifier.h"
#include "kis_debug.h"
#include "kis_config.h"
#include "kis_image_config.h"

//#define DEBUG_PYRAMID

#include <config-ocio.h>

#define ORIGINAL_INDEX           0
#define FIRST_NOT_ORIGINAL_INDEX 1
#define SCALE_FROM_INDEX(idx) (1./qreal(1<<(idx)))


/************* AUXILIARY FUNCTIONS **********************************/

#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>
#endif

#define ceiledSize(sz) QSize(ceil((sz).width()), ceil((sz).height()))
#define isOdd(x) ((x) & 0x01)

/**
 * Aligns @p value to the lowest integer not smaller than @p value and
 * that is a divident of alignment
 */
inline void alignByPow2Hi(qint32 &value, qint32 alignment)
{
    qint32 mask = alignment - 1;
    value |= mask;
    value++;
}

/**
 * Aligns @p value to the lowest integer not smaller than @p value and
 * that is, increased by one, a divident of alignment
 */
inline void alignByPow2ButOneHi(qint32 &value, qint32 alignment)
{
    qint32 mask = alignment - 1;
    value |= mask;
}

/**
 * Aligns @p value to the highest integer not exceeding @p value and
 * that is a divident of @p alignment
 */
inline void alignByPow2Lo(qint32 &value, qint32 alignment)
{
    qint32 mask = alignment - 1;
    value &= ~mask;
}

inline void alignRectBy2(qint32 &x, qint32 &y, qint32 &w, qint32 &h)
{
    x -= isOdd(x);
    y -= isOdd(y);
    w += isOdd(x);
    w += isOdd(w);
    h += isOdd(y);
    h += isOdd(h);
}


/************* class KisImagePyramid ********************************/

KisImagePyramid::KisImagePyramid(qint32 pyramidHeight)
        : m_monitorProfile(0)
        , m_monitorColorSpace(0)
        , m_pyramidHeight(pyramidHeight)
{
    configChanged();
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), this, SLOT(configChanged()));
}

KisImagePyramid::~KisImagePyramid()
{
    setImage(0);
}

void KisImagePyramid::setMonitorProfile(const KoColorProfile* monitorProfile,
                                        KoColorConversionTransformation::Intent renderingIntent,
                                        KoColorConversionTransformation::ConversionFlags conversionFlags)
{
    m_monitorProfile = monitorProfile;
    /**
     * If you change pixel size here, don't forget to change it
     * in optimized function downsamplePixels()
     */
    m_monitorColorSpace = KoColorSpaceRegistry::instance()->rgb8(monitorProfile);
    m_renderingIntent = renderingIntent;
    m_conversionFlags = conversionFlags;

    rebuildPyramid();
}

void KisImagePyramid::setChannelFlags(const QBitArray &channelFlags)
{
    m_channelFlags = channelFlags;
    int selectedChannels = 0;
    const KoColorSpace *projectionCs = m_originalImage->projection()->colorSpace();
    QList<KoChannelInfo*> channelInfo = projectionCs->channels();

    if (channelInfo.size() != m_channelFlags.size()) {
        m_channelFlags = QBitArray();
    }

    for (int i = 0; i < m_channelFlags.size(); ++i) {
        if (m_channelFlags.testBit(i) && channelInfo[i]->channelType() == KoChannelInfo::COLOR) {
            selectedChannels++;
            m_selectedChannelIndex = i;
        }
    }
    m_allChannelsSelected = (selectedChannels == m_channelFlags.size());
    m_onlyOneChannelSelected = (selectedChannels == 1);
}

void KisImagePyramid::setDisplayFilter(QSharedPointer<KisDisplayFilter> displayFilter)
{
    m_displayFilter = displayFilter;
}

void KisImagePyramid::rebuildPyramid()
{
    m_pyramid.clear();
    for (qint32 i = 0; i < m_pyramidHeight; i++) {
        m_pyramid.append(new KisPaintDevice(m_monitorColorSpace));
    }
}

void KisImagePyramid::clearPyramid()
{
    for (qint32 i = 0; i < m_pyramidHeight; i++) {
        m_pyramid[i]->clear();
    }
}

void KisImagePyramid::setImage(KisImageWSP newImage)
{
    if (newImage) {
        m_originalImage = newImage;

        clearPyramid();
        setImageSize(m_originalImage->width(), m_originalImage->height());

        // Get the full image size
        QRect rc = m_originalImage->projection()->exactBounds();

        KisImageConfig config(true);

        int patchWidth = config.updatePatchWidth();
        int patchHeight = config.updatePatchHeight();

        if (rc.width() * rc.height() <= patchWidth * patchHeight) {
            retrieveImageData(rc);
        }
        else {
            qint32 firstCol = rc.x() / patchWidth;
            qint32 firstRow = rc.y() / patchHeight;

            qint32 lastCol = (rc.x() + rc.width()) / patchWidth;
            qint32 lastRow = (rc.y() + rc.height()) / patchHeight;

            for(qint32 i = firstRow; i <= lastRow; i++) {
                for(qint32 j = firstCol; j <= lastCol; j++) {
                    QRect maxPatchRect(j * patchWidth,
                                       i * patchHeight,
                                       patchWidth, patchHeight);
                    QRect patchRect = rc & maxPatchRect;
                    retrieveImageData(patchRect);
                }
            }

        }
        //TODO: check whether there is needed recalculateCache()
    }
}

void KisImagePyramid::setImageSize(qint32 w, qint32 h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);
    /* nothing interesting */
}

void KisImagePyramid::updateCache(const QRect &dirtyImageRect)
{
    retrieveImageData(dirtyImageRect);
}

void KisImagePyramid::retrieveImageData(const QRect &rect)
{
    // XXX: use QThreadStorage to cache the two patches (512x512) of pixels. Note
    // that when we do that, we need to reset that cache when the projection's
    // colorspace changes.
    const KoColorSpace *projectionCs = m_originalImage->projection()->colorSpace();
    KisPaintDeviceSP originalProjection = m_originalImage->projection();
    quint32 numPixels = rect.width() * rect.height();

    QScopedArrayPointer<quint8> originalBytes(
        new quint8[originalProjection->colorSpace()->pixelSize() * numPixels]);

    originalProjection->readBytes(originalBytes.data(), rect);

    if (m_displayFilter &&
        m_useOcio &&
        projectionCs->colorModelId() == RGBAColorModelID) {

#ifdef HAVE_OCIO
        const KoColorProfile *destinationProfile =
            m_displayFilter->useInternalColorManagement() ?
            m_monitorProfile : projectionCs->profile();

        const KoColorSpace *floatCs =
            KoColorSpaceRegistry::instance()->colorSpace(
                RGBAColorModelID.id(),
                Float32BitsColorDepthID.id(),
                destinationProfile);

        const KoColorSpace *modifiedMonitorCs =
            KoColorSpaceRegistry::instance()->colorSpace(
                RGBAColorModelID.id(),
                Integer8BitsColorDepthID.id(),
                destinationProfile);

        if (projectionCs->colorDepthId() == Float32BitsColorDepthID) {
            m_displayFilter->filter(originalBytes.data(), numPixels);
        } else {
            QScopedArrayPointer<quint8> dst(new quint8[floatCs->pixelSize() * numPixels]);
            projectionCs->convertPixelsTo(originalBytes.data(), dst.data(), floatCs, numPixels, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
            m_displayFilter->filter(dst.data(), numPixels);
            originalBytes.swap(dst);
        }

        {
            QScopedArrayPointer<quint8> dst(new quint8[modifiedMonitorCs->pixelSize() * numPixels]);
            floatCs->convertPixelsTo(originalBytes.data(), dst.data(), modifiedMonitorCs, numPixels, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
            originalBytes.swap(dst);
        }
#endif
    }
    else {
        QList<KoChannelInfo*> channelInfo = projectionCs->channels();
        if (m_channelFlags.size() != channelInfo.size()) {
            setChannelFlags(QBitArray());
        }
        if (!m_channelFlags.isEmpty() && !m_allChannelsSelected) {
            QScopedArrayPointer<quint8> dst(new quint8[projectionCs->pixelSize() * numPixels]);

            KisConfig cfg(true);

            if (m_onlyOneChannelSelected && !cfg.showSingleChannelAsColor()) {
                projectionCs->convertChannelToVisualRepresentation(originalBytes.data(), dst.data(), numPixels, m_selectedChannelIndex);
            }
            else {
                projectionCs->convertChannelToVisualRepresentation(originalBytes.data(), dst.data(), numPixels, m_channelFlags);
            }
            originalBytes.swap(dst);
        }

        QScopedArrayPointer<quint8> dst(new quint8[m_monitorColorSpace->pixelSize() * numPixels]);
        projectionCs->convertPixelsTo(originalBytes.data(), dst.data(), m_monitorColorSpace, numPixels, m_renderingIntent, m_conversionFlags);
        originalBytes.swap(dst);
    }

    m_pyramid[ORIGINAL_INDEX]->writeBytes(originalBytes.data(), rect);
}

void KisImagePyramid::recalculateCache(KisPPUpdateInfoSP info)
{
    KisPaintDevice *src;
    KisPaintDevice *dst;
    QRect currentSrcRect = info->dirtyImageRectVar;

    for (int i = FIRST_NOT_ORIGINAL_INDEX; i < m_pyramidHeight; i++) {
        src = m_pyramid[i-1].data();
        dst = m_pyramid[i].data();
        if (!currentSrcRect.isEmpty()) {
            currentSrcRect = downsampleByFactor2(currentSrcRect, src, dst);
        }
    }

#ifdef DEBUG_PYRAMID
    QImage image = m_pyramid[ORIGINAL_INDEX]->convertToQImage(m_monitorProfile, m_renderingIntent, m_conversionFlags);
    image.save("./PYRAMID_BASE.png");

    image = m_pyramid[1]->convertToQImage(m_monitorProfile, m_renderingIntent, m_conversionFlags);
    image.save("./LEVEL1.png");

    image = m_pyramid[2]->convertToQImage(m_monitorProfile, m_renderingIntent, m_conversionFlags);
    image.save("./LEVEL2.png");
    image = m_pyramid[3]->convertToQImage(m_monitorProfile, m_renderingIntent, m_conversionFlags);
    image.save("./LEVEL3.png");
#endif
}

QRect KisImagePyramid::downsampleByFactor2(const QRect& srcRect,
        KisPaintDevice* src,
        KisPaintDevice* dst)
{
    qint32 srcX, srcY, srcWidth, srcHeight;
    srcRect.getRect(&srcX, &srcY, &srcWidth, &srcHeight);
    alignRectBy2(srcX, srcY, srcWidth, srcHeight);

    // Nothing to do
    if (srcWidth < 1) return QRect();
    if (srcHeight < 1) return QRect();

    qint32 dstX = srcX / 2;
    qint32 dstY = srcY / 2;
    qint32 dstWidth = srcWidth / 2;
    qint32 dstHeight = srcHeight / 2;

    KisHLineConstIteratorSP srcIt0 = src->createHLineConstIteratorNG(srcX, srcY, srcWidth);
    KisHLineConstIteratorSP srcIt1 = src->createHLineConstIteratorNG(srcX, srcY + 1, srcWidth);
    KisHLineIteratorSP dstIt = dst->createHLineIteratorNG(dstX, dstY, dstWidth);

    int conseqPixels = 0;
    for (int row = 0; row < dstHeight; ++row) {
        do {
            int srcItConseq = srcIt0->nConseqPixels();
            int dstItConseq = dstIt->nConseqPixels();
            conseqPixels = qMin(srcItConseq, dstItConseq * 2);

            Q_ASSERT(!isOdd(conseqPixels));

            downsamplePixels(srcIt0->oldRawData(), srcIt1->oldRawData(),
                             dstIt->rawData(), conseqPixels);


            srcIt1->nextPixels(conseqPixels);
            dstIt->nextPixels(conseqPixels / 2);
        } while (srcIt0->nextPixels(conseqPixels));
        srcIt0->nextRow();
        srcIt0->nextRow();
        srcIt1->nextRow();
        srcIt1->nextRow();
        dstIt->nextRow();
    }
    return QRect(dstX, dstY, dstWidth, dstHeight);
}

void  KisImagePyramid::downsamplePixels(const quint8 *srcRow0,
                                        const quint8 *srcRow1,
                                        quint8 *dstRow,
                                        qint32 numSrcPixels)
{
    /**
     * FIXME (mandatory): Use SSE and friends here.
     */

    qint16 b = 0;
    qint16 g = 0;
    qint16 r = 0;
    qint16 a = 0;

    static const qint32 pixelSize = 4; // This is preview argb8 mode

    for (qint32 i = 0; i < numSrcPixels / 2; i++) {
        b = srcRow0[0] + srcRow1[0] + srcRow0[4] + srcRow1[4];
        g = srcRow0[1] + srcRow1[1] + srcRow0[5] + srcRow1[5];
        r = srcRow0[2] + srcRow1[2] + srcRow0[6] + srcRow1[6];
        a = srcRow0[3] + srcRow1[3] + srcRow0[7] + srcRow1[7];

        dstRow[0] = b / 4;
        dstRow[1] = g / 4;
        dstRow[2] = r / 4;
        dstRow[3] = a / 4;

        dstRow += pixelSize;
        srcRow0 += 2 * pixelSize;
        srcRow1 += 2 * pixelSize;
    }
}

int KisImagePyramid::findFirstGoodPlaneIndex(qreal scale,
        QSize originalSize)
{
    qint32 nearest = 0;

    for (qint32 i = 0; i < m_pyramidHeight; i++) {
        qreal planeScale = SCALE_FROM_INDEX(i);
        if (planeScale < scale) {
            if (originalSize*scale == originalSize*planeScale)
                nearest = i;
            break;
        }
        nearest = i;
    }

    // FOR DEBUGGING
    //nearest = 0;
    //nearest = qMin(1, nearest);

    dbgRender << "First good plane:" << nearest << "(sc:" << scale << ")";
    return nearest;
}

void KisImagePyramid::alignSourceRect(QRect& rect, qreal scale)
{
    qint32 index = findFirstGoodPlaneIndex(scale, rect.size());
    qint32 alignment = 1 << index;

    dbgRender << "Before alignment:\t" << rect;

    /**
     * Assume that KisImage pixels are always positive
     * It allows us to use binary op-s for aligning
     */
    Q_ASSERT(rect.left() >= 0 && rect.top() >= 0);

    qint32 x1, y1, x2, y2;
    rect.getCoords(&x1, &y1, &x2, &y2);

    alignByPow2Lo(x1, alignment);
    alignByPow2Lo(y1, alignment);
    /**
     * Here is a workaround of Qt's QRect::right()/bottom()
     * "historical reasons". It should be one pixel smaller
     * than actual right/bottom position
     */
    alignByPow2ButOneHi(x2, alignment);
    alignByPow2ButOneHi(y2, alignment);

    rect.setCoords(x1, y1, x2, y2);

    dbgRender << "After alignment:\t" << rect;
}

KisImagePatch KisImagePyramid::getNearestPatch(KisPPUpdateInfoSP info)
{
    qint32 index = findFirstGoodPlaneIndex(qMax(info->scaleX, info->scaleY),
                                           info->imageRect.size());
    qreal planeScale = SCALE_FROM_INDEX(index);
    qint32 alignment = 1 << index;

    alignByPow2Hi(info->borderWidth, alignment);

    KisImagePatch patch(info->imageRect, info->borderWidth,
                        planeScale, planeScale);

    patch.setImage(convertToQImageFast(m_pyramid[index],
                                       patch.patchRect()));
    return patch;
}

void KisImagePyramid::drawFromOriginalImage(QPainter& gc, KisPPUpdateInfoSP info)
{
    KisImagePatch patch = getNearestPatch(info);
    patch.drawMe(gc, info->viewportRect, info->renderHints);
}

QImage KisImagePyramid::convertToQImageFast(KisPaintDeviceSP paintDevice,
        const QRect& unscaledRect)
{
    qint32 x, y, w, h;
    unscaledRect.getRect(&x, &y, &w, &h);

    QImage image = QImage(w, h, QImage::Format_ARGB32);

    paintDevice->dataManager()->readBytes(image.bits(), x, y, w, h);

    return image;
}

void KisImagePyramid::configChanged()
{
    KisConfig cfg(true);
    m_useOcio = cfg.useOcio();
}

