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

#include <KoProgressUpdater.h>
#include <KoUpdater.h>

#include <kis_layer.h>
#include <kis_math_toolbox.h>
#include <widgets/kis_multi_double_filter_widget.h>
#include <widgets/kis_multi_integer_filter_widget.h>
#include <kis_paint_device.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>

KisWaveletNoiseReduction::KisWaveletNoiseReduction()
        : KisFilter(id(), categoryEnhance(), i18n("&Wavelet Noise Reducer..."))
{
    setSupportsPainting(false);
    setSupportsIncrementalPainting(false);
    setSupportsThreading(false);
}


KisWaveletNoiseReduction::~KisWaveletNoiseReduction()
{
}

KisConfigWidget * KisWaveletNoiseReduction::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP, const KisImageWSP) const
{
    vKisDoubleWidgetParam param;
    param.push_back(KisDoubleWidgetParam(0.0, 256.0, BEST_WAVELET_THRESHOLD_VALUE, i18n("Threshold"), "threshold"));
    return new KisMultiDoubleFilterWidget(id().id(), parent, id().id(), param);
}

KisFilterConfiguration* KisWaveletNoiseReduction::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration* config = new KisFilterConfiguration(id().id(), 0);
    config->setProperty("threshold", BEST_WAVELET_THRESHOLD_VALUE);
    return config;
}

void KisWaveletNoiseReduction::process(KisPaintDeviceSP device,
                                      const QRect& applyRect,
                                      const KisFilterConfiguration* config,
                                      KoUpdater* progressUpdater
                                      ) const
{
    Q_ASSERT(device);
    // TODO take selections into account
    float threshold;

    if (!config) {
        config = defaultConfiguration(device);
    }

    threshold = config->getDouble("threshold", BEST_WAVELET_THRESHOLD_VALUE);

    qint32 depth = device->colorSpace()->colorChannelCount();

    int size;
    int maxrectsize = qMax(applyRect.width(), applyRect.height());
    for (size = 2; size < maxrectsize; size *= 2) ;

    KisMathToolbox* mathToolbox = KisMathToolboxRegistry::instance()->get(device->colorSpace()->mathToolboxId().id());

    if (progressUpdater) {
        progressUpdater->setRange(0, mathToolbox->fastWaveletTotalSteps(applyRect) * 2 + size*size*depth);
    }
    int count = 0;
//     connect(mathToolbox, SIGNAL(nextStep()), this, SLOT(incProgress()));


//     dbgFilters << size <<"" << maxrectsize <<"" << srcTopLeft.x() <<"" << srcTopLeft.y();

//     dbgFilters <<"Transforming...";
//     setProgressStage( i18n("Fast wavelet transformation") ,progress());
    KisMathToolbox::KisWavelet* buff = 0;
    KisMathToolbox::KisWavelet* wav = 0;

    try {
        buff = mathToolbox->initWavelet(device, applyRect);
    } catch (std::bad_alloc) {
        if (buff) delete buff;
        return;
    }
    try {
        wav = mathToolbox->fastWaveletTransformation(device, applyRect, buff);
    } catch (std::bad_alloc) {
        if (wav) delete wav;
        return;
    }

//     dbgFilters <<"Thresholding...";
    float* fin = wav->coeffs + wav->depth * wav->size * wav->size;

    for (float* it = wav->coeffs + wav->depth; it < fin; it++) {
        if (*it > threshold) {
            *it -= threshold;
        } else if (*it < -threshold) {
            *it += threshold;
        } else {
            *it = 0.;
        }
        if (progressUpdater) progressUpdater->setValue(++count);
    }

//     dbgFilters <<"Untransforming...";

    mathToolbox->fastWaveletUntransformation(device, applyRect, wav, buff);

    delete wav;
    delete buff;
//     disconnect(mathToolbox, SIGNAL(nextStep()), this, SLOT(incProgress()));
}
