/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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

#include "kis_simple_noise_reducer.h"

#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>

#include <kis_mask_generator.h>
#include <kis_iterators_pixel.h>
#include <kis_convolution_kernel.h>
#include <kis_convolution_painter.h>
#include <kis_global.h>
#include <widgets/kis_multi_integer_filter_widget.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>
#include <kis_paint_device.h>
#include <kis_selection.h>

KisSimpleNoiseReducer::KisSimpleNoiseReducer()
        : KisFilter(id(), categoryEnhance(), i18n("&Gaussian Noise Reduction"))
{
    setSupportsPainting(false);
    setSupportsPreview(true);
    setSupportsIncrementalPainting(false);
}

KisSimpleNoiseReducer::~KisSimpleNoiseReducer()
{
}

KisConfigWidget * KisSimpleNoiseReducer::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, const KisImageWSP image) const
{
    Q_UNUSED(dev);
    Q_UNUSED(image);
    vKisIntegerWidgetParam param;
    param.push_back(KisIntegerWidgetParam(0, 255, 15, i18n("Threshold"), "threshold"));
    param.push_back(KisIntegerWidgetParam(0, 10, 1, i18n("Window size"), "windowsize"));
    return new KisMultiIntegerFilterWidget(id().id(), parent, id().id(), param);
}

KisFilterConfiguration * KisSimpleNoiseReducer::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration* config = new KisFilterConfiguration(id().id(), 0);
    config->setProperty("threshold", 15);
    config->setProperty("windowsize", 1);
    return config;
}

inline int ABS(int v)
{
    if (v < 0) return -v;
    return v;
}

void KisSimpleNoiseReducer::process(KisConstProcessingInformation srcInfo,
                                    KisProcessingInformation dstInfo,
                                    const QSize& size,
                                    const KisFilterConfiguration* config,
                                    KoUpdater* progressUpdater
                                   ) const
{
    const KisPaintDeviceSP src = srcInfo.paintDevice();
    KisPaintDeviceSP dst = dstInfo.paintDevice();
    QPoint dstTopLeft = dstInfo.topLeft();
    QPoint srcTopLeft = srcInfo.topLeft();
    Q_ASSERT(!src.isNull());
    Q_ASSERT(!dst.isNull());

    int threshold, windowsize;
    if (config == 0) {
        config = defaultConfiguration(src);
    }
    if (progressUpdater) {
        progressUpdater->setRange(0, size.width() * size.height());
    }
    int count = 0;

    threshold = config->getInt("threshold", 15);
    windowsize = config->getInt("windowsize", 1);

    const KoColorSpace* cs = src->colorSpace();

    // Compute the blur mask
    KisCircleMaskGenerator* kas = new KisCircleMaskGenerator(2*windowsize + 1, 2*windowsize + 1, windowsize, windowsize);

    KisConvolutionKernelSP kernel = KisConvolutionKernel::fromMaskGenerator(kas);
    delete kas;

    KisPaintDeviceSP interm = new KisPaintDevice(*src); // TODO no need for a full copy and then a transaction
    KisConvolutionPainter painter(interm);
    painter.beginTransaction("bouuh");
    painter.applyMatrix(kernel, interm, srcTopLeft, dstTopLeft, size, BORDER_REPEAT);

    if (progressUpdater && progressUpdater->interrupted()) {
        return;
    }


    KisHLineIteratorPixel dstIt = dst->createHLineIterator(dstTopLeft.x(), dstTopLeft.y(), size.width(), dstInfo.selection());
    KisHLineConstIteratorPixel srcIt = src->createHLineConstIterator(srcTopLeft.x(), srcTopLeft.y(), size.width(), srcInfo.selection());
    KisHLineConstIteratorPixel intermIt = interm->createHLineConstIterator(srcTopLeft.x(), srcTopLeft.y(), size.width());

    for (int j = 0; j < size.height() && !(progressUpdater && progressUpdater->interrupted()); j++) {
        while (!srcIt.isDone() && !(progressUpdater && progressUpdater->interrupted())) {
            if (srcIt.isSelected()) {
                quint8 diff = cs->difference(srcIt.oldRawData(), intermIt.rawData());
                if (diff > threshold) {
                    memcpy(dstIt.rawData(), intermIt.rawData(), cs->pixelSize());
                } else {
                    memcpy(dstIt.rawData(), srcIt.oldRawData(), cs->pixelSize());
                }
            }
            if (progressUpdater) progressUpdater->setValue(++count);
            ++srcIt;
            ++dstIt;
            ++intermIt;
        }
        srcIt.nextRow();
        dstIt.nextRow();
        intermIt.nextRow();
    }

}

