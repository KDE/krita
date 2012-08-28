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

#include <kundo2command.h>

#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>

#include <kis_mask_generator.h>
#include <kis_convolution_kernel.h>
#include <kis_convolution_painter.h>
#include <kis_global.h>
#include <widgets/kis_multi_integer_filter_widget.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>
#include <kis_paint_device.h>
#include <kis_selection.h>
#include <kis_iterator_ng.h>

KisSimpleNoiseReducer::KisSimpleNoiseReducer()
        : KisFilter(id(), categoryEnhance(), i18n("&Gaussian Noise Reduction..."))
{
    setSupportsPainting(false);
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

void KisSimpleNoiseReducer::process(KisPaintDeviceSP device,
                                    const QRect& applyRect,
                                    const KisFilterConfiguration* config,
                                    KoUpdater* progressUpdater
                                   ) const
{
    QPoint srcTopLeft = applyRect.topLeft();
    Q_ASSERT(device);

    int threshold, windowsize;
    if (config == 0) {
        config = defaultConfiguration(device);
    }
    if (progressUpdater) {
        progressUpdater->setRange(0, applyRect.width() * applyRect.height());
    }
    int count = 0;

    threshold = config->getInt("threshold", 15);
    windowsize = config->getInt("windowsize", 1);

    const KoColorSpace* cs = device->colorSpace();

    // Compute the blur mask
    KisCircleMaskGenerator* kas = new KisCircleMaskGenerator(2*windowsize + 1, 1, windowsize, windowsize, 2);

    KisConvolutionKernelSP kernel = KisConvolutionKernel::fromMaskGenerator(kas);
    delete kas;

    KisPaintDeviceSP interm = new KisPaintDevice(*device); // TODO no need for a full copy and then a transaction
    KisConvolutionPainter painter(interm);
    painter.beginTransaction("bouuh");
    painter.applyMatrix(kernel, interm, srcTopLeft, srcTopLeft, applyRect.size(), BORDER_REPEAT);
    painter.deleteTransaction();

    if (progressUpdater && progressUpdater->interrupted()) {
        return;
    }


    KisHLineIteratorSP dstIt = device->createHLineIteratorNG(srcTopLeft.x(), srcTopLeft.y(), applyRect.width());
    KisHLineConstIteratorSP intermIt = interm->createHLineConstIteratorNG(srcTopLeft.x(), srcTopLeft.y(), applyRect.width());

    for (int j = 0; j < applyRect.height() && !(progressUpdater && progressUpdater->interrupted()); j++) {
        do {
                quint8 diff = cs->difference(dstIt->oldRawData(), intermIt->oldRawData());
                if (diff > threshold) {
                    memcpy(dstIt->rawData(), intermIt->oldRawData(), cs->pixelSize());
                }
            if (progressUpdater) progressUpdater->setValue(++count);
            intermIt->nextPixel();
        } while (dstIt->nextPixel() && !(progressUpdater && progressUpdater->interrupted()));
        dstIt->nextRow();
        intermIt->nextRow();
    }

}

