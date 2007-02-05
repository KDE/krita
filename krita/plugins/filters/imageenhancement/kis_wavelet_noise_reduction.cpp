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

#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_math_toolbox.h>
#include <kis_meta_registry.h>
#include <kis_multi_double_filter_widget.h>
#include <kis_paint_device.h>

KisWaveletNoiseReduction::KisWaveletNoiseReduction()
    : KisFilter(id(), "enhance", i18n("&Wavelet Noise Reducer"))
{
}


KisWaveletNoiseReduction::~KisWaveletNoiseReduction()
{
}

KisFilterConfigWidget * KisWaveletNoiseReduction::createConfigurationWidget(QWidget* parent, KisPaintDeviceSP )
{
    vKisDoubleWidgetParam param;
    param.push_back( KisDoubleWidgetParam( 0.0, 256.0, BEST_WAVELET_THRESHOLD_VALUE, i18n("Threshold"), "threshold" ) );
    return new KisMultiDoubleFilterWidget(id().id(), parent, id().id(), param );
}

KisFilterConfiguration* KisWaveletNoiseReduction::designerConfiguration(const KisPaintDeviceSP)
{
    KisFilterConfiguration* config = new KisFilterConfiguration(m_id.id(), 0);
    config->setProperty("threshold", BEST_WAVELET_THRESHOLD_VALUE);
    return config;
}

void KisWaveletNoiseReduction::process(const KisPaintDeviceSP src, const QPoint& srcTopLeft, KisPaintDeviceSP dst, const QPoint& dstTopLeft, const QSize& areaSize, KisFilterConfiguration* config)
{

    float threshold;

    if(!config)
    {
        config = defaultConfiguration(src);
    }

    threshold = config->getDouble("threshold", BEST_WAVELET_THRESHOLD_VALUE);

    qint32 depth = src->colorSpace()->colorChannelCount();

    int size;
    int maxrectsize = qMax( areaSize.width(), areaSize.height());
    for(size = 2; size < maxrectsize; size *= 2) ;

    KisMathToolbox* mathToolbox = KisMetaRegistry::instance()->mtRegistry()->get( src->colorSpace()->mathToolboxId() );
    QRect srcRect(srcTopLeft, areaSize);
    setProgressTotalSteps(mathToolbox->fastWaveletTotalSteps(srcRect) * 2 + size*size*depth );
    connect(mathToolbox, SIGNAL(nextStep()), this, SLOT(incProgress()));


//     kDebug(41005) << size << " " << maxrectsize << " " << srcTopLeft.x() << " " << srcTopLeft.y() << endl;

//     kDebug(41005) << "Transforming..." << endl;
    setProgressStage( i18n("Fast wavelet transformation") ,progress());
    KisMathToolbox::KisWavelet* buff = 0;
    KisMathToolbox::KisWavelet* wav = 0;
    try {
        buff = mathToolbox->initWavelet(src, srcRect);
    } catch(std::bad_alloc)
    {
        if(buff) delete buff;
        return;
    }
    try {
        wav = mathToolbox->fastWaveletTransformation(src, srcRect, buff);
    } catch(std::bad_alloc)
    {
        if(wav) delete wav;
        return;
    }

//     kDebug(41005) << "Thresholding..." << endl;
    float* fin = wav->coeffs + wav->depth*wav->size*wav->size;
    setProgressStage( i18n("Thresholding") ,progress());
    for(float* it = wav->coeffs + wav->depth; it < fin; it++)
    {
        if( *it > threshold)
        {
            *it -= threshold;
        } else if( *it < -threshold ) {
            *it += threshold;
        } else {
            *it = 0.;
        }
        incProgress();
    }

//     kDebug(41005) << "Untransforming..." << endl;

    setProgressStage( i18n("Fast wavelet untransformation") ,progress());
    mathToolbox->fastWaveletUntransformation( dst, QRect(dstTopLeft, areaSize ), wav, buff);

    delete wav;
    delete buff;
    disconnect(mathToolbox, SIGNAL(nextStep()), this, SLOT(incProgress()));

    setProgressDone(); // Must be called even if you don't really support progression
}
