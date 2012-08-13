/*
 *  Copyright (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
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
#include "kis_image_pyramid.h"

#include <KoCompositeOp.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>

#include "kis_display_filter.h"
#include "kis_painter.h"
#include "kis_iterator_ng.h"
#include "kis_datamanager.h"

#include "kis_debug.h"
#include "kis_config.h"

//#define DEBUG_PYRAMID

#include "config-ocio.h"
#ifdef HAVE_OCIO
#include <OpenColorIO/OpenColorIO.h>
#include <OpenColorIO/OpenColorTransforms.h>
#endif

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
 * Aligns @value to the lowest integer not smaller than @value and
 * that is a divident of alignment
 */
inline void alignByPow2Hi(qint32 &value, qint32 alignment)
{
    qint32 mask = alignment - 1;
    value |= mask;
    value++;
}

/**
 * Aligns @value to the lowest integer not smaller than @value and
 * that is, increased by one, a divident of alignment
 */
inline void alignByPow2ButOneHi(qint32 &value, qint32 alignment)
{
    qint32 mask = alignment - 1;
    value |= mask;
}

/**
 * Aligns @value to the highest integer not exceeding @value and
 * that is a divident of @alignment
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
        , m_displayFilter(0)
        , m_pyramidHeight(pyramidHeight)
{
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

void KisImagePyramid::setDisplayFilter(KisDisplayFilter *displayFilter)
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
        retrieveImageData(m_originalImage->projection()->exactBounds());
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

    quint8 *originalBytes = originalProjection->colorSpace()->allocPixelBuffer(numPixels);
    originalProjection->readBytes(originalBytes, rect);

    KisConfig cfg;

    if (m_displayFilter && cfg.useOcio() && projectionCs->colorModelId() == RGBAColorModelID) {
#ifdef HAVE_OCIO
        if (projectionCs->colorDepthId() == Float16BitsColorDepthID) {
            projectionCs = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), QString());

            float *dst = reinterpret_cast<float*>(projectionCs->allocPixelBuffer(numPixels));
            half *src = reinterpret_cast<half*>(originalBytes);

            for (quint32 i = 0; i < numPixels; ++i) {
                dst[i] = src[i];
            }
            delete[] originalBytes;
            originalBytes = reinterpret_cast<quint8*>(dst);

        }
        m_displayFilter->filter(originalBytes, originalBytes, numPixels);
#endif
    }

    quint8 *dstBytes = m_monitorColorSpace->allocPixelBuffer(numPixels);
    projectionCs->convertPixelsTo(originalBytes, dstBytes, m_monitorColorSpace, numPixels, m_renderingIntent, m_conversionFlags);

    m_pyramid[ORIGINAL_INDEX]->writeBytes(dstBytes, rect);

    delete[] originalBytes;
    delete[] dstBytes;
}

void KisImagePyramid::recalculateCache(KisPPUpdateInfoSP info)
{
    KisPaintDevice *src;
    KisPaintDevice *dst;
    QRect currentSrcRect = info->dirtyImageRect;

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
