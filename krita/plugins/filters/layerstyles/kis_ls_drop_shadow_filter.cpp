/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_ls_drop_shadow_filter.h"

#include <cstdlib>

#include <QBitArray>

#include <KoUpdater.h>

#include "psd.h"
#include "kis_psd_struct_converters.h"

#include "kis_convolution_kernel.h"
#include "kis_convolution_painter.h"
#include "kis_gaussian_kernel.h"

#include "kis_pixel_selection.h"
#include "kis_fill_painter.h"
#include "kis_iterator_ng.h"
#include "kis_random_accessor_ng.h"

#include "kis_psd_layer_style.h"


KisLsDropShadowFilter::KisLsDropShadowFilter(bool isDropShadow)
    : KisLayerStyleFilter(KoID("lsdropshadow", i18n("Drop Shadow (style)")))
    , m_isDropShadow(isDropShadow)
{
}

struct KisShadowPropertiesAdapter {

    KisShadowPropertiesAdapter(const psd_layer_effects_drop_shadow &shadow)
        : effect_enable(shadow.effect_enable)
        , blend_mode(shadow.blend_mode)
        , color(shadow.color)
        , native_color(shadow.native_color)
        , opacity(shadow.opacity)
        , angle(shadow.angle)
        , use_global_light(shadow.use_global_light)
        , distance(shadow.distance)
        , spread(shadow.spread)
        , size(shadow.size)
        , anti_aliased(shadow.anti_aliased)
        , noise(shadow.noise)
        , knocks_out(shadow.knocks_out)
        , inverts_selection(false)
        , edge_hidden(true)
        {
            for(int i = 0; i < PSD_LOOKUP_TABLE_SIZE; ++i) {
                contour_lookup_table[i] = shadow.contour_lookup_table[i];
            }
        }

    KisShadowPropertiesAdapter(const psd_layer_effects_inner_shadow &shadow)
        : effect_enable(shadow.effect_enable)
        , blend_mode(shadow.blend_mode)
        , color(shadow.color)
        , native_color(shadow.native_color)
        , opacity(shadow.opacity)
        , angle(shadow.angle)
        , use_global_light(shadow.use_global_light)
        , distance(shadow.distance)
        , spread(shadow.choke)
        , size(shadow.size)
        , anti_aliased(shadow.anti_aliased)
        , noise(shadow.noise)
        , knocks_out(true)
        , inverts_selection(true)
        , edge_hidden(false)
        {
            for(int i = 0; i < PSD_LOOKUP_TABLE_SIZE; ++i) {
                contour_lookup_table[i] = shadow.contour_lookup_table[i];
            }
        }

    QPoint calculateOffset(const psd_layer_effects_context *context) const {
        qint32 angle = this->use_global_light ?
            context->global_angle : this->angle;

        qint32 distance_x = -qRound(this->distance * cos(kisDegreesToRadians(qreal(angle))));
        qint32 distance_y =  qRound(this->distance * sin(kisDegreesToRadians(qreal(angle))));

        return QPoint(distance_x, distance_y);
    }

    bool effect_enable; // Effect enabled

    QString blend_mode; // already in Krita format!
    QColor color;
    QColor native_color;
    quint8 opacity; // Opacity as a percent (0...100)
    qint32 angle; // Angle in degrees
    bool use_global_light; // Use this angle in all of the layer effects
    qint32 distance; // Distance in pixels
    qint32 spread; // Intensity as a percent
    qint32 size; // Blur value in pixels

    quint8 contour_lookup_table[PSD_LOOKUP_TABLE_SIZE];
    bool anti_aliased;
    qint32 noise;
    bool knocks_out;
    bool inverts_selection;
    bool edge_hidden;
};

void findEdge(KisPixelSelectionSP selection, const QRect &applyRect, const bool edgeHidden)
{
    KisSequentialIterator dstIt(selection, applyRect);

    if (edgeHidden) {
        do {
            quint8 *pixelPtr = dstIt.rawData();

            *pixelPtr =
                (*pixelPtr < 24) ?
                *pixelPtr * 10 : 0xFF;

        } while(dstIt.nextPixel());
    } else {
        do {
            quint8 *pixelPtr = dstIt.rawData();
            *pixelPtr = 0xFF;

        } while(dstIt.nextPixel());
    }
}

void applyContourCorrection(KisPixelSelectionSP selection,
                            const QRect &applyRect,
                            const quint8 *lookup_table,
                            bool antiAliased,
                            bool edgeHidden)
{
    quint8 contour[PSD_LOOKUP_TABLE_SIZE] = {
        0x00, 0x0b, 0x16, 0x21, 0x2c, 0x37, 0x42, 0x4d, 0x58, 0x63, 0x6e, 0x79, 0x84, 0x8f, 0x9a, 0xa5,
        0xb0, 0xbb, 0xc6, 0xd1, 0xdc, 0xf2, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    if (edgeHidden) {
        if (antiAliased) {
            for (int i = 0; i < PSD_LOOKUP_TABLE_SIZE; i++) {
                contour[i] = contour[i] * lookup_table[i] >> 8;
            }
        } else {
            for (int i = 0; i < PSD_LOOKUP_TABLE_SIZE; i++) {
                contour[i] = contour[i] * lookup_table[(int)((int)(i / 2.55) * 2.55 + 0.5)] >> 8;
            }
        }
    } else {
        if (antiAliased) {
            for (int i = 0; i < PSD_LOOKUP_TABLE_SIZE; i++) {
                contour[i] = lookup_table[i];
            }
        } else {
            for (int i = 0; i < PSD_LOOKUP_TABLE_SIZE; i++) {
                contour[i] = lookup_table[(int)((int)(i / 2.55) * 2.55 + 0.5)];
            }
        }
    }

    KisSequentialIterator dstIt(selection, applyRect);
    do {
        quint8 *pixelPtr = dstIt.rawData();
        *pixelPtr = contour[*pixelPtr];
    } while(dstIt.nextPixel());
}

KisPixelSelectionSP generateRandomSelection(const QRect &rc)
{
    KisPixelSelectionSP selection = new KisPixelSelection();
    KisSequentialIterator dstIt(selection, rc);

    if (RAND_MAX >= 0x00FFFFFF) {
        do {
            int randValue = qrand();
            *dstIt.rawData() = (quint8) randValue;
            if (!dstIt.nextPixel()) break;

            randValue >>= 8;
            *dstIt.rawData() = (quint8) randValue;
            if (!dstIt.nextPixel()) break;

            randValue >>= 8;
            *dstIt.rawData() = (quint8) randValue;
        } while(dstIt.nextPixel());

    } else {
        do {
            *dstIt.rawData() = (quint8) rand();
        } while(dstIt.nextPixel());
    }

    return selection;
}

#define NOISE_NEED_BORDER 8

void applyNoise(KisPixelSelectionSP selection,
                const QRect &applyRect,
                int noise,
                const psd_layer_effects_context *context)
{
    Q_UNUSED(context);

    const QRect overlayRect = kisGrowRect(applyRect, NOISE_NEED_BORDER);

    KisPixelSelectionSP randomSelection = generateRandomSelection(overlayRect);
    KisPixelSelectionSP randomOverlay = new KisPixelSelection();

    KisSequentialConstIterator noiseIt(randomSelection, overlayRect);
    KisSequentialConstIterator srcIt(selection, overlayRect);
    KisRandomAccessorSP dstIt = randomOverlay->createRandomAccessorNG(overlayRect.x(), overlayRect.y());

    do {
        int itX = noiseIt.x();
        int itY = noiseIt.y();

        int x = itX + (*noiseIt.rawDataConst() >> 4) - 8;
        int y = itY + (*noiseIt.rawDataConst() & 0x0F) - 8;
        x = (x + itX) >> 1;
        y = (y + itY) >> 1;

        dstIt->moveTo(x, y);

        quint8 dstAlpha = *dstIt->rawData();
        quint8 srcAlpha = *srcIt.rawDataConst();

        int value = qMin(255, dstAlpha + srcAlpha);

        *dstIt->rawData() = value;

    } while(noiseIt.nextPixel() && srcIt.nextPixel());

    noise = noise * 255 / 100;

    KisPainter gc(selection);
    gc.setOpacity(noise);
    gc.setCompositeOp(COMPOSITE_COPY);
    gc.bitBlt(applyRect.topLeft(), randomOverlay, applyRect);
}

inline QRect growRectFromRadius(const QRect &rc, int radius)
{
    int halfSize = KisGaussianKernel::kernelSizeFromRadius(radius) / 2;
    return rc.adjusted(-halfSize, -halfSize, halfSize, halfSize);
}

void applyGaussian(KisPixelSelectionSP selection,
                   const QRect &applyRect,
                   qreal radius)
{
    KisGaussianKernel::applyGaussian(selection, applyRect,
                                     radius, radius,
                                     QBitArray(), 0);
}

KisSelectionSP selectionFromAlphaChannel(KisPaintDeviceSP device,
                                         const QRect &srcRect)
{
    const KoColorSpace *cs = device->colorSpace();

    KisSelectionSP baseSelection = new KisSelection(new KisSelectionEmptyBounds(0));
    KisPixelSelectionSP selection = baseSelection->pixelSelection();

    KisSequentialConstIterator srcIt(device, srcRect);
    KisSequentialIterator dstIt(selection, srcRect);

    do {
        quint8 *dstPtr = dstIt.rawData();
        const quint8* srcPtr = srcIt.rawDataConst();
        *dstPtr = cs->opacityU8(srcPtr);
    } while(srcIt.nextPixel() && dstIt.nextPixel());

    return baseSelection;
}

struct ShadowRectsData
{
    enum Direction {
        NEED_RECT,
        CHANGE_RECT
    };

    ShadowRectsData(const QRect &applyRect,
                    const psd_layer_effects_context *context,
                    const KisShadowPropertiesAdapter *shadow_adapter,
                    Direction direction)
    {
        spread_size = (shadow_adapter->spread * shadow_adapter->size + 50) / 100;
        blur_size = shadow_adapter->size - spread_size;
        offset = shadow_adapter->calculateOffset(context);

        // need rect calculation in reverse order
        dstRect = applyRect;

        const int directionCoeff = direction == NEED_RECT ? -1 : 1;
        srcRect = dstRect.translated(directionCoeff * offset);

        noiseNeedRect = shadow_adapter->noise > 0 ?
            kisGrowRect(srcRect, NOISE_NEED_BORDER) : srcRect;

        blurNeedRect = blur_size ?
            growRectFromRadius(noiseNeedRect, blur_size) : noiseNeedRect;

        spreadNeedRect = spread_size ?
            growRectFromRadius(blurNeedRect, spread_size) : blurNeedRect;

        // qDebug() << ppVar(dstRect);
        // qDebug() << ppVar(srcRect);
        // qDebug() << ppVar(noiseNeedRect);
        // qDebug() << ppVar(blurNeedRect);
        // qDebug() << ppVar(spreadNeedRect);
    }

    inline QRect finalNeedRect() const {
        return spreadNeedRect;
    }

    inline QRect finalChangeRect() const {
        // TODO: is it correct?
        return spreadNeedRect;
    }

    qint32 spread_size;
    qint32 blur_size;
    QPoint offset;

    QRect srcRect;
    QRect dstRect;
    QRect noiseNeedRect;
    QRect blurNeedRect;
    QRect spreadNeedRect;
};


void applyDropShadow(KisPaintDeviceSP srcDevice,
                     KisPaintDeviceSP dstDevice,
                     const QRect &applyRect,
                     const psd_layer_effects_context *context,
                     const KisShadowPropertiesAdapter *shadow_adapter)
{
    if (applyRect.isEmpty()) return;

    ShadowRectsData d(applyRect, context, shadow_adapter, ShadowRectsData::NEED_RECT);

    KisSelectionSP baseSelection =
        selectionFromAlphaChannel(srcDevice, d.spreadNeedRect);

    KisPixelSelectionSP selection = baseSelection->pixelSelection();

    //selection->convertToQImage(0, QRect(0,0,300,300)).save("0_selection_initial.png");

    if (shadow_adapter->inverts_selection) {
        selection->invert();
    }

    /**
     * Copy selection which will be erased from the original later
     */
    KisPixelSelectionSP knockOutSelection;
    if (shadow_adapter->knocks_out) {
        knockOutSelection = new KisPixelSelection(*selection);
    }

    /**
     * Spread and blur the selection
     */
    if (d.spread_size) {
        applyGaussian(selection, d.blurNeedRect, d.spread_size);

        // TODO: find out why in libpsd we pass false here. If we do so,
        //       the result is fully black, which is not expected
        findEdge(selection, d.blurNeedRect, true /*shadow_adapter->edge_hidden*/);
    }

    //selection->convertToQImage(0, QRect(0,0,300,300)).save("1_selection_spread.png");

    if (d.blur_size) {
        applyGaussian(selection, d.noiseNeedRect, d.blur_size);
    }
    //selection->convertToQImage(0, QRect(0,0,300,300)).save("2_selection_blur.png");

    /**
     * Contour correction
     */
    applyContourCorrection(selection,
                           d.noiseNeedRect,
                           shadow_adapter->contour_lookup_table,
                           shadow_adapter->anti_aliased,
                           shadow_adapter->edge_hidden);

    //selection->convertToQImage(0, QRect(0,0,300,300)).save("3_selection_contour.png");

    /**
     * Noise
     */
    if (shadow_adapter->noise > 0) {
        applyNoise(selection,
                   d.srcRect,
                   shadow_adapter->noise,
                   context);
    }
    //selection->convertToQImage(0, QRect(0,0,300,300)).save("4_selection_noise.png");

    selection->setX(d.offset.x());
    selection->setY(d.offset.y());

    /**
     * Knock-out original outline of the device from the resulting shade
     */
    if (shadow_adapter->knocks_out) {
        KIS_ASSERT_RECOVER_RETURN(knockOutSelection);

        QRect knockOutRect = !shadow_adapter->inverts_selection ?
            d.srcRect : d.spreadNeedRect;

        knockOutRect &= d.dstRect;

        KisPainter gc(selection);
        gc.setCompositeOp(COMPOSITE_ERASE);
        gc.bitBlt(knockOutRect.topLeft(), knockOutSelection, knockOutRect);
    }
    //selection->convertToQImage(0, QRect(0,0,300,300)).save("5_selection_knockout.png");

    const KoColor shadowColor(shadow_adapter->color, srcDevice->colorSpace());

    KisPaintDeviceSP tempDevice;
    if (srcDevice == dstDevice) {
        if (context->keep_original) {
            tempDevice = new KisPaintDevice(*srcDevice);
        }
        srcDevice->clear(d.srcRect);
    } else {
        tempDevice = srcDevice;
    }

    {
        QRect shadowRect(d.dstRect);
        KisFillPainter gc(dstDevice);

        const QString compositeOp = shadow_adapter->blend_mode;
        const quint8 opacityU8 = 255.0 / 100.0 * shadow_adapter->opacity;
        gc.setCompositeOp(compositeOp);
        gc.setOpacity(opacityU8);

        gc.setSelection(baseSelection);
        gc.fillSelection(shadowRect, shadowColor);
        gc.end();
    }

    //dstDevice->convertToQImage(0, QRect(0,0,300,300)).save("6_device_shadow.png");

    if (context->keep_original) {
        KisPainter gc(dstDevice);
        gc.bitBlt(d.dstRect.topLeft(), tempDevice, d.dstRect);
    }
}

QSharedPointer<KisShadowPropertiesAdapter>
KisLsDropShadowFilter::createAdapter(KisPSDLayerStyleSP style) const
{
    QSharedPointer<KisShadowPropertiesAdapter> shadowAdapter;

    if (m_isDropShadow) {
        shadowAdapter = toQShared(new KisShadowPropertiesAdapter(*style->drop_shadow()));
    } else {
        shadowAdapter = toQShared(new KisShadowPropertiesAdapter(*style->inner_shadow()));
    }

    return shadowAdapter;
}

void KisLsDropShadowFilter::processDirectly(KisPaintDeviceSP src,
                                            KisPaintDeviceSP dst,
                                            const QRect &applyRect,
                                            KisPSDLayerStyleSP style) const
{
    KIS_ASSERT_RECOVER_RETURN(style);
    QSharedPointer<KisShadowPropertiesAdapter> shadowAdapter = createAdapter(style);
    if (!shadowAdapter->effect_enable) return;

    applyDropShadow(src, dst, applyRect, style->context(), shadowAdapter.data());
}

QRect KisLsDropShadowFilter::neededRect(const QRect &rect, KisPSDLayerStyleSP style) const
{
    QSharedPointer<KisShadowPropertiesAdapter> shadowAdapter = createAdapter(style);
    if (!shadowAdapter->effect_enable) return rect;

    ShadowRectsData d(rect, style->context(), shadowAdapter.data(), ShadowRectsData::NEED_RECT);
    return rect | d.finalNeedRect();
}

QRect KisLsDropShadowFilter::changedRect(const QRect &rect, KisPSDLayerStyleSP style) const
{
    QSharedPointer<KisShadowPropertiesAdapter> shadowAdapter = createAdapter(style);
    if (!shadowAdapter->effect_enable) return rect;

    ShadowRectsData d(rect, style->context(), shadowAdapter.data(), ShadowRectsData::CHANGE_RECT);
    return style->context()->keep_original ?
        d.finalChangeRect() : rect | d.finalChangeRect();
}
