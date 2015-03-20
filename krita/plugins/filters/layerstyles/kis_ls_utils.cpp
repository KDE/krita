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

#include <KoAbstractGradient.h>
#include <KoColorSpace.h>

#include "psd.h"

#include "kis_default_bounds.h"
#include "kis_pixel_selection.h"
#include "kis_random_accessor_ng.h"
#include "kis_iterator_ng.h"

#include "kis_convolution_kernel.h"
#include "kis_convolution_painter.h"
#include "kis_gaussian_kernel.h"

#include "kis_fill_painter.h"


namespace KisLsUtils
{

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

    QRect growRectFromRadius(const QRect &rc, int radius)
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

    namespace Private {
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
            JitterGradientIndex(const QRect &applyRect, int jitter)
                : randomSelection(generateRandomSelection(applyRect)),
                  noiseIt(randomSelection, applyRect),
                  m_jitterCoeff(jitter * 255 / 100)
                {
                }

            int popOneIndex(int selectionAlpha) {
                int gradientIndex = 255 - selectionAlpha;
                gradientIndex += m_jitterCoeff * *noiseIt.rawDataConst() >> 8;
                gradientIndex &= 0xFF;

                if (selectionAlpha != 0 && selectionAlpha != 255) {
                    qDebug() << ppVar(255 - selectionAlpha) << ppVar(gradientIndex);
                }

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

                do {
                    quint8 selAlpha = *selIt.rawDataConst();
                    int gradientIndex = indexFetcher.popOneIndex(selAlpha);
                    const KoColor &color = table[gradientIndex];
                    quint8 tableAlpha = color.opacityU8();

                    memcpy(dstIt.rawData(), color.data(), pixelSize);

                    if (selAlpha < 24 && tableAlpha == 255) {
                        tableAlpha = int(selAlpha) * 10 * tableAlpha >> 8;
                        cs->setOpacity(dstIt.rawData(), tableAlpha, 1);
                    }

                } while(selIt.nextPixel() &&
                        dstIt.nextPixel() &&
                        indexFetcher.nextPixel());

            } else {

                do {
                    int gradientIndex = indexFetcher.popOneIndex(*selIt.rawDataConst());
                    const KoColor &color = table[gradientIndex];
                    memcpy(dstIt.rawData(), color.data(), pixelSize);
                } while(selIt.nextPixel() &&
                        dstIt.nextPixel() &&
                        indexFetcher.nextPixel());

            }
        }

        void applyGradient(KisPaintDeviceSP device,
                           KisPixelSelectionSP selection,
                           const QRect &applyRect,
                           const QVector<KoColor> &table,
                           bool edgeHidden,
                           int jitter)
        {
            if (!jitter) {
                LinearGradientIndex fetcher;
                applyGradientImpl(device, selection, applyRect, table, edgeHidden, fetcher);
            } else {
                JitterGradientIndex fetcher(applyRect, jitter);
                applyGradientImpl(device, selection, applyRect, table, edgeHidden, fetcher);
            }
        }
    }

    const int noiseNeedBorder = 8;

    void applyNoise(KisPixelSelectionSP selection,
                    const QRect &applyRect,
                    int noise,
                    const psd_layer_effects_context *context)
    {
        Q_UNUSED(context);

        const QRect overlayRect = kisGrowRect(applyRect, noiseNeedBorder);

        KisPixelSelectionSP randomSelection = Private::generateRandomSelection(overlayRect);
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

    void applyFinalSelection(KisSelectionSP baseSelection,
                             KisPaintDeviceSP srcDevice,
                             KisPaintDeviceSP dstDevice,
                             const QRect &srcRect,
                             const QRect &dstRect,
                             const psd_layer_effects_context *context,
                             const psd_layer_effects_shadow_base *config)
    {
        const KoColor effectColor(config->color(), srcDevice->colorSpace());

        KisPaintDeviceSP tempDevice;
        if (srcDevice == dstDevice) {
            if (context->keep_original) {
                tempDevice = new KisPaintDevice(*srcDevice);
            }
            srcDevice->clear(srcRect);
        } else {
            tempDevice = srcDevice;
        }

        const QRect effectRect(dstRect);
        const QString compositeOp = config->blendMode();
        const quint8 opacityU8 = 255.0 / 100.0 * config->opacity();

        if (config->fillType() == psd_fill_solid_color) {
            KisFillPainter gc(dstDevice);
            gc.setCompositeOp(compositeOp);
            gc.setOpacity(opacityU8);

            gc.setSelection(baseSelection);
            gc.fillSelection(effectRect, effectColor);
            gc.end();

        } else if (config->fillType() == psd_fill_gradient) {
            KisPaintDeviceSP overlayDevice =
                new KisPaintDevice(dstDevice->colorSpace());

            QVector<KoColor> table(256);
            Private::getGradientTable(config->gradient(), &table, dstDevice->colorSpace());

            Private::applyGradient(overlayDevice, baseSelection->pixelSelection(),
                                   effectRect, table,
                                   true, config->jitter());

            KisPainter gc(dstDevice);
            gc.setCompositeOp(compositeOp);
            gc.setOpacity(opacityU8);

            gc.bitBlt(effectRect.topLeft(), overlayDevice, effectRect);
            gc.end();
        }

        //dstDevice->convertToQImage(0, QRect(0,0,300,300)).save("6_device_shadow.png");

        if (context->keep_original) {
            KisPainter gc(dstDevice);
            gc.bitBlt(dstRect.topLeft(), tempDevice, dstRect);
        }
    }
}
