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

#include <compositeops/KoVcMultiArchBuildSupport.h> //MSVC requires that Vc come first
#include "kis_simple_noise_reducer.h"

#include <kundo2command.h>

#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include <KoUpdater.h>

#include <kis_mask_generator.h>
#include <kis_convolution_kernel.h>
#include <kis_convolution_painter.h>
#include <kis_global.h>
#include <widgets/kis_multi_integer_filter_widget.h>
#include <filter/kis_filter_category_ids.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>
#include <kis_paint_device.h>
#include <kis_selection.h>
#include <KisSequentialIteratorProgress.h>
#include "kis_lod_transform.h"


KisSimpleNoiseReducer::KisSimpleNoiseReducer()
    : KisFilter(id(), FiltersCategoryEnhanceId, i18n("&Gaussian Noise Reduction..."))
{
    setSupportsPainting(false);
    setSupportsLevelOfDetail(true);
}

KisSimpleNoiseReducer::~KisSimpleNoiseReducer()
{
}

KisConfigWidget * KisSimpleNoiseReducer::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool) const
{
    Q_UNUSED(dev);
    vKisIntegerWidgetParam param;
    param.push_back(KisIntegerWidgetParam(0, 255, 15, i18n("Threshold"), "threshold"));
    param.push_back(KisIntegerWidgetParam(0, 10, 1, i18n("Window size"), "windowsize"));
    return new KisMultiIntegerFilterWidget(id().id(), parent, id().id(), param);
}

KisFilterConfigurationSP  KisSimpleNoiseReducer::defaultConfiguration() const
{
    KisFilterConfigurationSP config = factoryConfiguration();
    config->setProperty("threshold", 15);
    config->setProperty("windowsize", 1);
    return config;
}

inline int ABS(int v)
{
    if (v < 0) return -v;
    return v;
}

void KisSimpleNoiseReducer::processImpl(KisPaintDeviceSP device,
                                        const QRect& applyRect,
                                        const KisFilterConfigurationSP _config,
                                        KoUpdater* progressUpdater
                                        ) const
{
    QPoint srcTopLeft = applyRect.topLeft();
    Q_ASSERT(device);

    KisFilterConfigurationSP config = _config ? _config : defaultConfiguration();

    const int threshold = config->getInt("threshold", 15);
    const int windowsize = config->getInt("windowsize", 1);

    const KoColorSpace* cs = device->colorSpace();

    // Compute the blur mask
    KisCircleMaskGenerator* kas = new KisCircleMaskGenerator(2*windowsize + 1, 1, windowsize, windowsize, 2, true);

    KisConvolutionKernelSP kernel = KisConvolutionKernel::fromMaskGenerator(kas);
    delete kas;

    KisPaintDeviceSP interm = new KisPaintDevice(*device); // TODO no need for a full copy and then a transaction
    KisConvolutionPainter painter(interm);
    painter.beginTransaction();
    painter.applyMatrix(kernel, interm, srcTopLeft, srcTopLeft, applyRect.size(), BORDER_REPEAT);
    painter.deleteTransaction();


    KisSequentialConstIteratorProgress intermIt(interm, applyRect, progressUpdater);
    KisSequentialIterator dstIt(device, applyRect);

    while (dstIt.nextPixel() && intermIt.nextPixel()) {
        const quint8 diff = cs->difference(dstIt.oldRawData(), intermIt.oldRawData());
        if (diff > threshold) {
            memcpy(dstIt.rawData(), intermIt.oldRawData(), cs->pixelSize());
        }
    }
}

QRect KisSimpleNoiseReducer::neededRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const
{
    KisLodTransformScalar t(lod);

    const int windowsize = _config->getInt("windowsize", 1);
    const int margin  = qCeil(t.scale(qreal(windowsize))) + 1;
    return kisGrowRect(rect, margin);
}

QRect KisSimpleNoiseReducer::changedRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const
{
    return neededRect(rect, _config, lod);
}
