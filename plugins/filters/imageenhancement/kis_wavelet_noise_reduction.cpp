/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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

#include "kis_wavelet_noise_reduction.h"


#include <cmath>

#include <KoUpdater.h>

#include <kis_layer.h>
#include <kis_math_toolbox.h>
#include <widgets/kis_multi_double_filter_widget.h>
#include <widgets/kis_multi_integer_filter_widget.h>
#include <kis_paint_device.h>
#include <filter/kis_filter_category_ids.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>
#include "kis_global.h"

KisWaveletNoiseReduction::KisWaveletNoiseReduction()
    : KisFilter(id(), FiltersCategoryEnhanceId, i18n("&Wavelet Noise Reducer..."))
{
    setSupportsPainting(false);
    setSupportsThreading(false);
}


KisWaveletNoiseReduction::~KisWaveletNoiseReduction()
{
}

KisConfigWidget * KisWaveletNoiseReduction::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP, bool) const
{
    vKisDoubleWidgetParam param;
    param.push_back(KisDoubleWidgetParam(0.0, 256.0, BEST_WAVELET_THRESHOLD_VALUE, i18n("Threshold"), "threshold"));
    return new KisMultiDoubleFilterWidget(id().id(), parent, id().id(), param);
}

KisFilterConfigurationSP KisWaveletNoiseReduction::factoryConfiguration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration(id().id(), 0);
    config->setProperty("threshold", BEST_WAVELET_THRESHOLD_VALUE);
    return config;
}

void KisWaveletNoiseReduction::processImpl(KisPaintDeviceSP device,
                                           const QRect& applyRect,
                                           const KisFilterConfigurationSP _config,
                                           KoUpdater* progressUpdater
                                           ) const
{
    Q_ASSERT(device);

    KisFilterConfigurationSP config = _config ? _config : defaultConfiguration();
    const float threshold = config->getDouble("threshold", BEST_WAVELET_THRESHOLD_VALUE);

    KisMathToolbox mathToolbox;

    //     dbgFilters << size <<"" << maxrectsize <<"" << srcTopLeft.x() <<"" << srcTopLeft.y();

    //     dbgFilters <<"Transforming...";
    KisMathToolbox::KisWavelet* buff = 0;
    KisMathToolbox::KisWavelet* wav = 0;

    try {
        buff = mathToolbox.initWavelet(device, applyRect);
    } catch (const std::bad_alloc&) {
        if (buff) delete buff;
        return;
    }
    try {
        wav = mathToolbox.fastWaveletTransformation(device, applyRect, buff);
    } catch (const std::bad_alloc&) {
        if (wav) delete wav;
        return;
    }

    float* const fin = wav->coeffs + wav->depth * pow2(wav->size);
    float* const begin = wav->coeffs + wav->depth;

    const int size = fin - begin;
    const int progressOffset = int(std::ceil(std::log2(size / 100)));
    const int progressMask = (1 << progressOffset) - 1;
    const int numProgressSteps = size >> progressOffset;
    int pointsProcessed = 0;

    progressUpdater->setRange(0, numProgressSteps);

    for (float* it = begin; it < fin; it++) {
        if (*it > threshold) {
            *it -= threshold;
        } else if (*it < -threshold) {
            *it += threshold;
        } else {
            *it = 0.;
        }

        if (!(pointsProcessed & progressMask)) {
            progressUpdater->setValue(pointsProcessed >> progressOffset);
        }
        pointsProcessed++;
    }

    mathToolbox.fastWaveletUntransformation(device, applyRect, wav, buff);

    delete wav;
    delete buff;
}
