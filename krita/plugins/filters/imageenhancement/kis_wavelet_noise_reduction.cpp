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
#include <kis_paint_device_impl.h>

KisWaveletNoiseReduction::KisWaveletNoiseReduction()
    : KisFilter(id(), "enhance", i18n("&Wavelet Noise Reducer"))
{
}


KisWaveletNoiseReduction::~KisWaveletNoiseReduction()
{
}

KisFilterConfigWidget * KisWaveletNoiseReduction::createConfigurationWidget(QWidget* parent, KisPaintDeviceImplSP )
{
    vKisDoubleWidgetParam param;
    param.push_back( KisDoubleWidgetParam( 0.0, 256.0, BEST_WAVELET_THRESHOLD_VALUE, i18n("Threshold") ) );
    return new KisMultiDoubleFilterWidget(parent, id().id().ascii(), id().id().ascii(), param );
}

KisFilterConfiguration* KisWaveletNoiseReduction::configuration(QWidget* nwidget, KisPaintDeviceImplSP )
{
    KisMultiDoubleFilterWidget* widget = (KisMultiDoubleFilterWidget*) nwidget;
    if( widget == 0 )
    {
        return new KisWaveletNoiseReductionConfiguration( BEST_WAVELET_THRESHOLD_VALUE );
    } else {
        return new KisWaveletNoiseReductionConfiguration( widget->valueAt( 0 ) );
    }
}

void KisWaveletNoiseReduction::process(KisPaintDeviceImplSP src, KisPaintDeviceImplSP dst, KisFilterConfiguration* config, const QRect& rect)
{
    
    float threshold;
    if(config !=0)
    {
        KisWaveletNoiseReductionConfiguration* configWNRC = (KisWaveletNoiseReductionConfiguration*)config;
        threshold = configWNRC->threshold;
    } else {
        threshold = 1.0;
    }

    
    Q_INT32 depth = src -> colorSpace() -> nColorChannels();
    
    int size;
    int maxrectsize = (rect.height() < rect.width()) ? rect.width() : rect.height();
    for(size = 2; size < maxrectsize; size *= 2) ;
    
    KisMathToolbox* mathToolbox = KisMetaRegistry::instance()->mtRegistry()->get( src->colorSpace()->mathToolboxID() );
    setProgressTotalSteps(mathToolbox->fastWaveletTotalSteps(rect) * 2 + size*size*depth );
    connect(mathToolbox, SIGNAL(nextStep()), this, SLOT(incProgress()));
            
    
    kdDebug() << size << " " << maxrectsize << " " << rect.x() << " " << rect.y() << endl;
    
    kdDebug() << "Transforming..." << endl;
    setProgressStage( i18n("Fast wavelet transformation") ,progress());
    KisMathToolbox::KisWavelet* buff = mathToolbox->initWavelet(src, rect);
    KisMathToolbox::KisWavelet* wav = mathToolbox->fastWaveletTransformation(src, rect, buff);
    
    kdDebug() << "Thresholding..." << endl;
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
    
    kdDebug() << "Untransforming..." << endl;
    
    setProgressStage( i18n("Fast wavelet untransformation") ,progress());
    mathToolbox->fastWaveletUntransformation( dst, rect, wav, buff);
    
    delete wav;
    delete buff;
    disconnect(mathToolbox, SIGNAL(nextStep()), this, SLOT(incProgress()));
    
    setProgressDone(); // Must be called even if you don't really support progression
}
