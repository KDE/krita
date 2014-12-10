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

#include <KoProgressUpdater.h>
#include <KoUpdater.h>

#include "psd.h"
#include "kis_psd_struct_converters.h"

#include "filter/kis_filter_configuration.h"

#include "kis_convolution_kernel.h"
#include "kis_convolution_painter.h"
#include "kis_gaussian_kernel.h"

#include "kis_pixel_selection.h"
#include "kis_fill_painter.h"
#include "kis_iterator_ng.h"
#include "kis_random_accessor_ng.h"


KisLsDropShadowFilter::KisLsDropShadowFilter() : KisFilter(id(), categoryEnhance(), i18n("Drop Shadow (style)..."))
{
    setSupportsPainting(false);
    setSupportsAdjustmentLayers(true);
    setColorSpaceIndependence(FULLY_INDEPENDENT);
}

KisConfigWidget * KisLsDropShadowFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP) const
{
    return 0;
}

KisFilterConfiguration* KisLsDropShadowFilter::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration* config = new KisFilterConfiguration(id().id(), 1);

    psd_layer_effects_drop_shadow dropShadow;
    dropShadow.effect_enable = true;
    KisPSD::dropShadowToConfig(&dropShadow, config);

    psd_layer_effects_context context;
    KisPSD::contextToConfig(&context, config);

    return config;
}

template <bool edgeHidden>
void findEdge(KisPixelSelectionSP selection)
{

    QRect applyRect = selection->selectedExactRect();

    KisSequentialIterator dstIt(selection, applyRect);
    do {
        quint8 *pixelPtr = dstIt.rawData();

        *pixelPtr =
            (edgeHidden && *pixelPtr < 24) ?
            *pixelPtr * 10 : 0xFF;

    } while(dstIt.nextPixel());
}

template <bool edgeHidden>
void applyContourCorrection(KisPixelSelectionSP selection,
                            const quint8 *lookup_table,
                            bool antiAliased)
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

    QRect applyRect = selection->selectedExactRect();

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

void applyNoise(KisPixelSelectionSP selection,
                int noise,
                const psd_layer_effects_context *context,
                KoUpdater* progressUpdater)
{
    QRect applyRect = selection->selectedExactRect();

    Q_UNUSED(context);
    Q_UNUSED(progressUpdater);
    KisPixelSelectionSP randomSelection = generateRandomSelection(applyRect);
    KisPixelSelectionSP randomOverlay = new KisPixelSelection();

    KisSequentialConstIterator noiseIt(randomSelection, applyRect);
    KisSequentialConstIterator srcIt(selection, applyRect);
    KisRandomAccessorSP dstIt = randomOverlay->createRandomAccessorNG(applyRect.x(), applyRect.y());

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

void applyGaussian(KisPixelSelectionSP selection,
                   qreal radius,
                   KoUpdater* progressUpdater)
{
    int halfSize = KisGaussianKernel::kernelSizeFromRadius(radius) / 2;
    QRect applyRect = selection->selectedExactRect();
    applyRect.adjust(-halfSize, -halfSize, halfSize, halfSize);

    KisGaussianKernel::applyGaussian(selection, applyRect,
                                     radius, radius,
                                     QBitArray(), progressUpdater);
}

KisSelectionSP selectionFromAlphaChannel(KisPaintDeviceSP device,
                                         const QRect &srcRect)
{
    const KoColorSpace *cs = device->colorSpace();

    KisSelectionSP baseSelection = new KisSelection();
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

/**
 * TODO:
 *
 * 1) Make it apply inside requested filtered rectangle only.  It also
 *    includes avoiding calls to exactBounds() and selectedExactRect()
 *    from everywhere.
 *
 * 2) Add an option for context->keep_original, so that the filter
 *    could be either applied separately or using layer ltyles
 *    framework
 *
 * 3) Move util functions into a separate compilation unit.
 */

void applyDropShadow(KisPaintDeviceSP device,
                     const psd_layer_effects_context *context,
                     const psd_layer_effects_drop_shadow *drop_shadow,
                     KoUpdater* progressUpdater)
{
    const QRect srcRect = device->exactBounds();
    if (srcRect.isEmpty()) return;

    KisSelectionSP baseSelection =
        selectionFromAlphaChannel(device, srcRect);

    KisPixelSelectionSP selection = baseSelection->pixelSelection();

    //selection->convertToQImage(0, QRect(0,0,300,300)).save("0_selection_initial.png");

    /**
     * Copy selection which will be erased from the original later
     */
    KisPixelSelectionSP knockOutSelection;
    if (drop_shadow->knocks_out) {
        knockOutSelection = new KisPixelSelection(*selection);
    }

    /**
     * Spread and blur the selection
     */
    qint32 spread_size = (drop_shadow->spread * drop_shadow->size + 50) / 100;
    qint32 blur_size = drop_shadow->size - spread_size;
    if(spread_size) {
        applyGaussian(selection, spread_size, progressUpdater);
        findEdge<true>(selection);
    }
    //selection->convertToQImage(0, QRect(0,0,300,300)).save("1_selection_spread.png");
    if(blur_size) {
        applyGaussian(selection, blur_size, progressUpdater);
    }
    //selection->convertToQImage(0, QRect(0,0,300,300)).save("2_selection_blur.png");

    /**
     * Contour correction
     */
    applyContourCorrection<true>(selection,
                                 drop_shadow->contour_lookup_table,
                                 drop_shadow->anti_aliased);
    //selection->convertToQImage(0, QRect(0,0,300,300)).save("3_selection_contour.png");

    /**
     * Noise
     */
    if (drop_shadow->noise > 0) {
        applyNoise(selection,
                   drop_shadow->noise,
                   context,
                   progressUpdater);
    }
    //selection->convertToQImage(0, QRect(0,0,300,300)).save("4_selection_noise.png");

    /**
     * Knock-out original outline of the device from the resulting shade
     */
    if (drop_shadow->knocks_out) {
        KIS_ASSERT_RECOVER_RETURN(knockOutSelection);

        QRect knockOutRect = knockOutSelection->selectedExactRect();
        KisPainter gc(selection);
        gc.setCompositeOp(COMPOSITE_ERASE);
        gc.bitBlt(knockOutRect.topLeft(), knockOutSelection, knockOutRect);
    }
    //selection->convertToQImage(0, QRect(0,0,300,300)).save("5_selection_knockout.png");

    const QPoint offset = drop_shadow->calculateOffset(context);
    const KoColor shadowColor(drop_shadow->color, device->colorSpace());

    selection->setX(offset.x());
    selection->setY(offset.y());


    // TODO: add keep_original switch here

    KisPaintDeviceSP tempDevice = new KisPaintDevice(*device);
    device->clear();

    {
        QRect shadowRect(selection->selectedExactRect());
        KisFillPainter gc(device);

        // TODO: should apply somewhere in the stack!
        const QString compositeOp = drop_shadow->blend_mode;
        const quint8 opacityU8 = 255.0 / 100.0 * drop_shadow->opacity;
        gc.setCompositeOp(compositeOp);
        gc.setOpacity(opacityU8);

        gc.setSelection(baseSelection);
        gc.fillSelection(shadowRect, shadowColor);
        gc.end();
    }

    //device->convertToQImage(0, QRect(0,0,300,300)).save("6_device_shadow.png");

    {
        KisPainter gc(device);
        gc.bitBlt(srcRect.topLeft(), tempDevice, srcRect);
    }
}

void KisLsDropShadowFilter::processImpl(KisPaintDeviceSP device,
                                       const QRect& applyRect,
                                       const KisFilterConfiguration* config,
                                       KoUpdater* progressUpdater) const
{
    KIS_ASSERT_RECOVER_RETURN(config);

    psd_layer_effects_drop_shadow dropShadow;
    KisPSD::configToDropShadow(config, &dropShadow);

    psd_layer_effects_context context;
    KisPSD::configToContext(config, &context);

    applyDropShadow(device, &context, &dropShadow, progressUpdater);


    // Just a reference on how to restore progress reporting
#if 0
    QPointer<KoUpdater> filterUpdater = 0;
    QPointer<KoUpdater> convolutionUpdater = 0;
    KoProgressUpdater* updater = 0;

    if (progressUpdater) {
        updater = new KoProgressUpdater(progressUpdater);
        updater->start(100, i18n("Unsharp Mask"));
        // Two sub-sub tasks that each go from 0 to 100.
        convolutionUpdater = updater->startSubtask();
        filterUpdater = updater->startSubtask();
    }

    if (!config) config = new KisFilterConfiguration(id().id(), 1);

    delete updater;

    if (progressUpdater) progressUpdater->setProgress(100);

#endif /* 0 */
}

QRect KisLsDropShadowFilter::neededRect(const QRect & rect, const KisFilterConfiguration* config) const
{
    psd_layer_effects_context context;
    KisPSD::configToContext(config, &context);

    psd_layer_effects_drop_shadow dropShadow;
    KisPSD::configToDropShadow(config, &dropShadow);

    const QPoint offset = dropShadow.calculateOffset(&context);

    QRect dstRect = rect;
    dstRect.adjust(dropShadow.size, dropShadow.size,
                   -dropShadow.size, -dropShadow.size);
    dstRect.translate(-offset);

    return rect | dstRect;
}

QRect KisLsDropShadowFilter::changedRect(const QRect & rect, const KisFilterConfiguration* config) const
{
    psd_layer_effects_context context;
    KisPSD::configToContext(config, &context);

    psd_layer_effects_drop_shadow dropShadow;
    KisPSD::configToDropShadow(config, &dropShadow);

    const QPoint offset = dropShadow.calculateOffset(&context);

    QRect dstRect = rect;
    dstRect.adjust(-dropShadow.size, -dropShadow.size,
                   dropShadow.size, dropShadow.size);
    dstRect.translate(offset);

    return context.keep_original ? dstRect : rect | dstRect;
}
