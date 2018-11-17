/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_ls_utils.h"

#include <resources/KoAbstractGradient.h>
#include <KoColorSpace.h>
#include <resources/KoPattern.h>


#include "psd.h"

#include "kis_default_bounds.h"
#include "kis_pixel_selection.h"
#include "kis_random_accessor_ng.h"
#include "kis_iterator_ng.h"

#include "kis_convolution_kernel.h"
#include "kis_convolution_painter.h"
#include "kis_gaussian_kernel.h"

#include "kis_fill_painter.h"
#include "kis_gradient_painter.h"
#include "kis_layer_style_filter_environment.h"
#include "kis_selection_filters.h"
#include "kis_multiple_projection.h"


namespace KisLsUtils
{

    QRect growSelectionUniform(KisPixelSelectionSP selection, int growSize, const QRect &applyRect)
    {
        QRect changeRect = applyRect;

        if (growSize > 0) {
            KisGrowSelectionFilter filter(growSize, growSize);
            changeRect = filter.changeRect(applyRect);
            filter.process(selection, applyRect);
        } else if (growSize < 0) {
            KisShrinkSelectionFilter filter(qAbs(growSize), qAbs(growSize), false);
            changeRect = filter.changeRect(applyRect);
            filter.process(selection, applyRect);
        }

        return changeRect;
    }

    KisSelectionSP selectionFromAlphaChannel(KisPaintDeviceSP device,
                                             const QRect &srcRect)
    {
        const KoColorSpace *cs = device->colorSpace();

        KisSelectionSP baseSelection = new KisSelection(new KisSelectionEmptyBounds(0));
        KisPixelSelectionSP selection = baseSelection->pixelSelection();

        KisSequentialConstIterator srcIt(device, srcRect);
        KisSequentialIterator dstIt(selection, srcRect);

        while (srcIt.nextPixel() && dstIt.nextPixel()) {
            quint8 *dstPtr = dstIt.rawData();
            const quint8* srcPtr = srcIt.rawDataConst();
            *dstPtr = cs->opacityU8(srcPtr);
        }

        return baseSelection;
    }

    void findEdge(KisPixelSelectionSP selection, const QRect &applyRect, const bool edgeHidden)
    {
        KisSequentialIterator dstIt(selection, applyRect);

        if (edgeHidden) {
            while(dstIt.nextPixel()) {
                quint8 *pixelPtr = dstIt.rawData();

                *pixelPtr =
                    (*pixelPtr < 24) ?
                    *pixelPtr * 10 : 0xFF;

            }
        } else {
            while(dstIt.nextPixel()) {
                quint8 *pixelPtr = dstIt.rawData();
                *pixelPtr = 0xFF;

            }
        }
    }

    QRect growRectFromRadius(const QRect &rc, int radius)
    {
        int halfSize = KisGaussianKernel::kernelSizeFromRadius(radius) / 2;
        return rc.adjusted(-halfSize, -halfSize, halfSize, halfSize);
    }

    void applyGaussianWithTransaction(KisPixelSelectionSP selection,
                                      const QRect &applyRect,
                                      qreal radius)
    {
        KisGaussianKernel::applyGaussian(selection, applyRect,
                                         radius, radius,
                                         QBitArray(), 0, true);
    }

    namespace Private {
        void getGradientTable(const KoAbstractGradient *gradient,
                              QVector<KoColor> *table,
                              const KoColorSpace *colorSpace)
        {
            KIS_ASSERT_RECOVER_RETURN(table->size() == 256);

            for (int i = 0; i < 256; i++) {
                gradient->colorAt(((*table)[i]), qreal(i) / 255.0);
                (*table)[i].convertTo(colorSpace);
            }
        }

        struct LinearGradientIndex
        {
            int popOneIndex(int selectionAlpha) {
                return 255 - selectionAlpha;
            }

            bool nextPixel() {
                return true;
            }
        };

        struct JitterGradientIndex
        {
            JitterGradientIndex(const QRect &applyRect,
                                int jitter,
                                const KisLayerStyleFilterEnvironment *env)
                : randomSelection(env->cachedRandomSelection(applyRect)),
                  noiseIt(randomSelection, applyRect),
                  m_jitterCoeff(jitter * 255 / 100)
                {
                }

            int popOneIndex(int selectionAlpha) {
                int gradientIndex = 255 - selectionAlpha;
                gradientIndex += m_jitterCoeff * *noiseIt.rawDataConst() >> 8;
                gradientIndex &= 0xFF;

                return gradientIndex;
            }

            bool nextPixel() {
                return noiseIt.nextPixel();
            }

        private:
            KisPixelSelectionSP randomSelection;
            KisSequentialConstIterator noiseIt;
            int m_jitterCoeff;
        };

        template <class IndexFetcher>
        void applyGradientImpl(KisPaintDeviceSP device,
                               KisPixelSelectionSP selection,
                               const QRect &applyRect,
                               const QVector<KoColor> &table,
                               bool edgeHidden,
                               IndexFetcher &indexFetcher)
        {
            KIS_ASSERT_RECOVER_RETURN(
                *table.first().colorSpace() == *device->colorSpace());

            const KoColorSpace *cs = device->colorSpace();
            const int pixelSize = cs->pixelSize();

            KisSequentialConstIterator selIt(selection, applyRect);
            KisSequentialIterator dstIt(device, applyRect);

            if (edgeHidden) {

                while (selIt.nextPixel() &&
                       dstIt.nextPixel() &&
                       indexFetcher.nextPixel()) {

                    quint8 selAlpha = *selIt.rawDataConst();
                    int gradientIndex = indexFetcher.popOneIndex(selAlpha);
                    const KoColor &color = table[gradientIndex];
                    quint8 tableAlpha = color.opacityU8();

                    memcpy(dstIt.rawData(), color.data(), pixelSize);

                    if (selAlpha < 24 && tableAlpha == 255) {
                        tableAlpha = int(selAlpha) * 10 * tableAlpha >> 8;
                        cs->setOpacity(dstIt.rawData(), tableAlpha, 1);
                    }

                }

            } else {

                while (selIt.nextPixel() &&
                       dstIt.nextPixel() &&
                       indexFetcher.nextPixel()) {

                    int gradientIndex = indexFetcher.popOneIndex(*selIt.rawDataConst());
                    const KoColor &color = table[gradientIndex];
                    memcpy(dstIt.rawData(), color.data(), pixelSize);
                }
            }
        }

        void applyGradient(KisPaintDeviceSP device,
                           KisPixelSelectionSP selection,
                           const QRect &applyRect,
                           const QVector<KoColor> &table,
                           bool edgeHidden,
                           int jitter,
                           const KisLayerStyleFilterEnvironment *env)
        {
            if (!jitter) {
                LinearGradientIndex fetcher;
                applyGradientImpl(device, selection, applyRect, table, edgeHidden, fetcher);
            } else {
                JitterGradientIndex fetcher(applyRect, jitter, env);
                applyGradientImpl(device, selection, applyRect, table, edgeHidden, fetcher);
            }
        }
    }

    const int noiseNeedBorder = 8;

    void applyNoise(KisPixelSelectionSP selection,
                    const QRect &applyRect,
                    int noise,
                    const psd_layer_effects_context *context,
                    const KisLayerStyleFilterEnvironment *env)
    {
        Q_UNUSED(context);

        const QRect overlayRect = kisGrowRect(applyRect, noiseNeedBorder);

        KisPixelSelectionSP randomSelection = env->cachedRandomSelection(overlayRect);
        KisPixelSelectionSP randomOverlay = new KisPixelSelection();

        KisSequentialConstIterator noiseIt(randomSelection, overlayRect);
        KisSequentialConstIterator srcIt(selection, overlayRect);
        KisRandomAccessorSP dstIt = randomOverlay->createRandomAccessorNG(overlayRect.x(), overlayRect.y());

        while (noiseIt.nextPixel() && srcIt.nextPixel()) {
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
        }

        noise = noise * 255 / 100;

        KisPainter gc(selection);
        gc.setOpacity(noise);
        gc.setCompositeOp(COMPOSITE_COPY);
        gc.bitBlt(applyRect.topLeft(), randomOverlay, applyRect);
    }

    //const int FULL_PERCENT_RANGE = 100;

    void adjustRange(KisPixelSelectionSP selection, const QRect &applyRect, const int range)
    {
        KIS_ASSERT_RECOVER_RETURN(range >= 1 && range <= 100);

        quint8 rangeTable[256];
        for(int i = 0; i < 256; i ++) {
            quint8 value = i * 100 / range;
            rangeTable[i] = qMin(value, quint8(255));
        }

        KisSequentialIterator dstIt(selection, applyRect);

        while (dstIt.nextPixel()) {
            quint8 *pixelPtr = dstIt.rawData();
            *pixelPtr = rangeTable[*pixelPtr];
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
        while (dstIt.nextPixel()) {
            quint8 *pixelPtr = dstIt.rawData();
            *pixelPtr = contour[*pixelPtr];
        }
    }

    void knockOutSelection(KisPixelSelectionSP selection,
                           KisPixelSelectionSP knockOutSelection,
                           const QRect &srcRect,
                           const QRect &dstRect,
                           const QRect &totalNeedRect,
                           const bool knockOutInverted)
    {
        KIS_ASSERT_RECOVER_RETURN(knockOutSelection);

        QRect knockOutRect = !knockOutInverted ? srcRect : totalNeedRect;
        knockOutRect &= dstRect;

        KisPainter gc(selection);
        gc.setCompositeOp(COMPOSITE_ERASE);
        gc.bitBlt(knockOutRect.topLeft(), knockOutSelection, knockOutRect);
    }

    void fillPattern(KisPaintDeviceSP fillDevice,
                     const QRect &applyRect,
                     KisLayerStyleFilterEnvironment *env,
                     int scale,
                     KoPatternSP pattern,
                     int horizontalPhase,
                     int verticalPhase,
                     bool alignWithLayer)
    {
        if (scale != 100) {
            warnKrita << "KisLsOverlayFilter::applyOverlay(): Pattern scaling is NOT implemented!";
        }

        QSize psize(pattern->width(), pattern->height());

        QPoint patternOffset(qreal(psize.width()) * horizontalPhase / 100,
                             qreal(psize.height()) * verticalPhase / 100);

        const QRect boundsRect = alignWithLayer ?
            env->layerBounds() : env->defaultBounds();

        patternOffset += boundsRect.topLeft();

        patternOffset.rx() %= psize.width();
        patternOffset.ry() %= psize.height();

        QRect fillRect = applyRect | applyRect.translated(patternOffset);

        KisFillPainter gc(fillDevice);
        gc.fillRect(fillRect.x(), fillRect.y(),
                    fillRect.width(), fillRect.height(), pattern, -patternOffset);
        gc.end();
    }

    void fillOverlayDevice(KisPaintDeviceSP fillDevice,
                           const QRect &applyRect,
                           const psd_layer_effects_overlay_base *config,
                           KisLayerStyleFilterEnvironment *env)
    {
        if (config->fillType() == psd_fill_solid_color) {
            KoColor color(config->color(), fillDevice->colorSpace());
            fillDevice->setDefaultPixel(color);

        } else if (config->fillType() == psd_fill_pattern) {
            fillPattern(fillDevice, applyRect, env,
                        config->scale(), config->pattern(),
                        config->horizontalPhase(),
                        config->verticalPhase(),
                        config->alignWithLayer());
        } else if (config->fillType() == psd_fill_gradient) {
            const QRect boundsRect = config->alignWithLayer() ?
                env->layerBounds() : env->defaultBounds();

            QPoint center = boundsRect.center();
            center += QPoint(boundsRect.width() * config->gradientXOffset() / 100,
                             boundsRect.height() * config->gradientYOffset() / 100);

            int width = (boundsRect.width() * config->scale() + 100) / 200;
            int height = (boundsRect.height() * config->scale() + 100) / 200;

            /* copy paste from libpsd */

            int angle = config->angle();
            int corner_angle = (int)(atan((qreal)boundsRect.height() / boundsRect.width()) * 180 / M_PI + 0.5);
            int sign_x = 1;
            int sign_y = 1;


            if(angle < 0) {
                angle += 360;
            }

            if (angle >= 90 && angle < 180) {
                angle = 180 - angle;
                sign_x = -1;
            } else if (angle >= 180 && angle < 270) {
                angle = angle - 180;
                sign_x = -1;
                sign_y = -1;
            } else if (angle >= 270 && angle <= 360) {
                angle = 360 - angle;
                sign_y = -1;
            }

            int radius_x = 0;
            int radius_y = 0;

            if (angle <= corner_angle) {
                radius_x = width;
                radius_y = (int)(radius_x * tan(kisDegreesToRadians(qreal(angle))) + 0.5);
            } else {
                radius_y = height;
                radius_x = (int)(radius_y / tan(kisDegreesToRadians(qreal(angle))) + 0.5);
            }

            int radius_corner = (int)(std::sqrt((qreal)(radius_x * radius_x + radius_y * radius_y)) + 0.5);

            /* end of copy paste from libpsd */

            KisGradientPainter gc(fillDevice);
            gc.setGradient(config->gradient());
            QPointF gradStart;
            QPointF gradEnd;
            KisGradientPainter::enumGradientRepeat repeat =
                KisGradientPainter::GradientRepeatNone;

            QPoint rectangularOffset(sign_x * radius_x, -sign_y * radius_y);


            switch(config->style())
            {
            case psd_gradient_style_linear:
                gc.setGradientShape(KisGradientPainter::GradientShapeLinear);
                repeat = KisGradientPainter::GradientRepeatNone;
                gradStart = center - rectangularOffset;
                gradEnd = center + rectangularOffset;
                break;

            case psd_gradient_style_radial:
                gc.setGradientShape(KisGradientPainter::GradientShapeRadial);
                repeat = KisGradientPainter::GradientRepeatNone;
                gradStart = center;
                gradEnd = center + QPointF(radius_corner, 0);
                break;

            case psd_gradient_style_angle:
                gc.setGradientShape(KisGradientPainter::GradientShapeConical);
                repeat = KisGradientPainter::GradientRepeatNone;
                gradStart = center;
                gradEnd = center + rectangularOffset;
                break;

            case psd_gradient_style_reflected:
                gc.setGradientShape(KisGradientPainter::GradientShapeLinear);
                repeat = KisGradientPainter::GradientRepeatAlternate;
                gradStart = center - rectangularOffset;
                gradEnd = center;
                break;

            case psd_gradient_style_diamond:
                gc.setGradientShape(KisGradientPainter::GradientShapeBiLinear);
                repeat = KisGradientPainter::GradientRepeatNone;
                gradStart = center - rectangularOffset;
                gradEnd = center + rectangularOffset;
                break;
            default:
                qFatal("Gradient Overlay: unknown switch case!");
                break;
            }

            gc.paintGradient(gradStart, gradEnd,
                             repeat, 0.0,
                             config->reverse(),
                             applyRect);
        }
    }

    void applyFinalSelection(const QString &projectionId,
                             KisSelectionSP baseSelection,
                             KisPaintDeviceSP srcDevice,
                             KisMultipleProjection *dst,
                             const QRect &/*srcRect*/,
                             const QRect &dstRect,
                             const psd_layer_effects_context */*context*/,
                             const psd_layer_effects_shadow_base *config,
                             const KisLayerStyleFilterEnvironment *env)
    {
        const KoColor effectColor(config->color(), srcDevice->colorSpace());

        const QRect effectRect(dstRect);
        const QString compositeOp = config->blendMode();
        const quint8 opacityU8 = 255.0 / 100.0 * config->opacity();
        KisPaintDeviceSP dstDevice = dst->getProjection(projectionId, compositeOp, opacityU8, QBitArray(), srcDevice);

        if (config->fillType() == psd_fill_solid_color) {
            KisFillPainter gc(dstDevice);
            gc.setCompositeOp(COMPOSITE_COPY);
            gc.setSelection(baseSelection);
            gc.fillSelection(effectRect, effectColor);
            gc.end();

        } else if (config->fillType() == psd_fill_gradient) {
            if (!config->gradient()) {
                warnKrita << "KisLsUtils::applyFinalSelection: Gradient object is null! Skipping...";
                return;
            }

            QVector<KoColor> table(256);
            Private::getGradientTable(config->gradient().data(), &table, dstDevice->colorSpace());

            Private::applyGradient(dstDevice, baseSelection->pixelSelection(),
                                   effectRect, table,
                                   true, config->jitter(), env);
        }

        //dstDevice->convertToQImage(0, QRect(0,0,300,300)).save("6_device_shadow.png");
    }

    bool checkEffectEnabled(const psd_layer_effects_shadow_base *config, KisMultipleProjection *dst)
    {
        bool result = config->effectEnabled();

        if (!result) {
            dst->freeAllProjections();
        }

        return result;
    }
}
