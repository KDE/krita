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

#include "kis_paint_device_impl.h"
#include "kis_layer.h"
#include <kis_iterators_pixel.h>
#include "kis_multi_double_filter_widget.h"

struct Wavelet {
    Wavelet(uint s, uint d) : coeffs(new float[s*s*d]), size(s), depth(d) { }
    ~Wavelet() { delete coeffs; }
    float* coeffs;
    uint size;
    uint depth;
};

KisWaveletNoiseReduction::KisWaveletNoiseReduction()
    : KisFilter(id(), "enhance", "&Wavelet Noise Reducer")
{
}


KisWaveletNoiseReduction::~KisWaveletNoiseReduction()
{
}

KisFilterConfigWidget * KisWaveletNoiseReduction::createConfigurationWidget(QWidget* parent, KisPaintDeviceImplSP )
{
    vKisDoubleWidgetParam param;
    param.push_back( KisDoubleWidgetParam( 0.0, 256.0, 10., i18n("Threshold") ) );
    return new KisMultiDoubleFilterWidget(parent, id().id().ascii(), id().id().ascii(), param );
}

KisFilterConfiguration* KisWaveletNoiseReduction::configuration(QWidget* nwidget, KisPaintDeviceImplSP )
{
    KisMultiDoubleFilterWidget* widget = (KisMultiDoubleFilterWidget*) nwidget;
    if( widget == 0 )
    {
        return new KisWaveletNoiseReductionConfiguration( 1.0);
    } else {
        return new KisWaveletNoiseReductionConfiguration( widget->valueAt( 0 ) );
    }
}

void wavetrans(Wavelet* wav, Wavelet* buff, uint halfsize)
{
    uint l = (2*halfsize)*wav->depth*sizeof(float);
    for(uint i = 0; i < halfsize; i++)
    {
        float * itLL = buff->coeffs + i*buff->size*buff->depth;
        float * itHL = buff->coeffs + (i*buff->size + halfsize)*buff->depth;
        float * itLH = buff->coeffs + (halfsize+i)*buff->size*buff->depth;
        float * itHH = buff->coeffs + ( (halfsize+i)*buff->size + halfsize)*buff->depth;
        float * itS11 = wav->coeffs + 2*i*wav->size*wav->depth;
        float * itS12 = wav->coeffs + (2*i*wav->size+1)*wav->depth;
        float * itS21 = wav->coeffs + (2*i+1)*wav->size*wav->depth;
        float * itS22 = wav->coeffs + ((2*i+1)*wav->size+1)*wav->depth;
        for(uint j = 0; j < halfsize; j++)
        {
            for( uint k = 0; k < wav->depth; k++)
            {
                *(itLL++) = (*itS11 + *itS12 + *itS21 + *itS22) * M_SQRT1_2;
                *(itHL++) = (*itS11 - *itS12 + *itS21 - *itS22) * M_SQRT1_2;
                *(itLH++) = (*itS11 + *itS12 - *itS21 - *itS22) * M_SQRT1_2;
                *(itHH++) = (*(itS11++) - *(itS12++) - *(itS21++) + *(itS22++)) * M_SQRT1_2;
            }
            itS11 += wav->depth; itS12 += wav->depth;
            itS21 += wav->depth; itS22 += wav->depth;
        }
    }
    for(uint i = 0; i < halfsize; i++)
    {
        uint p = i*wav->size*wav->depth;
        memcpy(wav->coeffs + p, buff->coeffs + p, l);
        p = (i + halfsize )*wav->size*wav->depth;
        memcpy(wav->coeffs + p, buff->coeffs + p, l);
    }
    if(halfsize != 1)
    {
        wavetrans(wav, buff, halfsize/2);
    }
}

#if 0
void saveToFile(KisPaintDeviceImplSP lay, QString fn)
{
    int w = lay->image()->width();
    int h = lay->image()->height();
    QImage img(w, h, 32);
    
    for(int j = 0; j < w; j++)
    {
        KisHLineIteratorPixel srcIt = lay->createHLineIterator(0, j, w , false);
        uint *p = (uint*)img.scanLine(j);
        while(!srcIt.isDone())
        {
            Q_UINT8* L = srcIt.rawData();
            *p = qRgb(L[0],L[1],L[2]);
            ++p; ++srcIt;
        }
    }
    img.save(fn, "PNG");
}
#endif

void waveuntrans(Wavelet* wav, Wavelet* buff, uint halfsize, KisColorSpace* cs)
{
#if 0
    KisImageSP intermImage = new KisImage(0, wav->size, wav->size, cs, "intermImage.NoiseReduction");
    Q_CHECK_PTR(intermImage);
    KisLayerSP intermLay = new KisLayer(intermImage, intermImage -> nextLayerName(), OPACITY_OPAQUE);
    intermImage -> add(intermLay, -1);

    for(int i = 0; i < wav->size; i++)
    {
        KisHLineIteratorPixel dstIt = intermLay->createHLineIterator(00, i, wav->size, true );
        float *srcIt = wav->coeffs + i*wav->size*wav->depth;
        while( ! dstIt.isDone() )
        {
            Q_UINT8* v1 = dstIt.rawData();
            for( int k = 0; k < wav->depth; k++)
            {
                v1[k] = (int)*srcIt;
                ++srcIt;
            }
            ++dstIt;
        }
    }
    saveToFile((KisPaintDeviceImplSP)intermLay, QString("test_utr_%1.png").arg(halfsize));
#endif

    uint l = (2*halfsize)*wav->depth*sizeof(float);
    for(uint i = 0; i < halfsize; i++)
    {
        float * itLL = wav->coeffs + i*buff->size*buff->depth;
        float * itHL = wav->coeffs + (i*buff->size + halfsize)*buff->depth;
        float * itLH = wav->coeffs + (halfsize+i)*buff->size*buff->depth;
        float * itHH = wav->coeffs + ( (halfsize+i)*buff->size + halfsize)*buff->depth;
        float * itS11 = buff->coeffs + 2*i*wav->size*wav->depth;
        float * itS12 = buff->coeffs + (2*i*wav->size+1)*wav->depth;
        float * itS21 = buff->coeffs + (2*i+1)*wav->size*wav->depth;
        float * itS22 = buff->coeffs + ((2*i+1)*wav->size+1)*wav->depth;
        for(uint j = 0; j < halfsize; j++)
        {
            for( uint k = 0; k < wav->depth; k++)
            {
                *(itS11++) = (*itLL + *itHL + *itLH + *itHH)*0.25*M_SQRT2;
                *(itS12++) = (*itLL - *itHL + *itLH - *itHH)*0.25*M_SQRT2;
                *(itS21++) = (*itLL + *itHL - *itLH - *itHH)*0.25*M_SQRT2;
                *(itS22++) = (*(itLL++) - *(itHL++) - *(itLH++) + *(itHH++))*0.25*M_SQRT2;
            }
            itS11 += wav->depth; itS12 += wav->depth;
            itS21 += wav->depth; itS22 += wav->depth;
        }
    }
    for(uint i = 0; i < halfsize; i++)
    {
        uint p = i*wav->size*wav->depth;
        memcpy(wav->coeffs + p, buff->coeffs + p, l);
        p = (i + halfsize )*wav->size*wav->depth;
        memcpy(wav->coeffs + p, buff->coeffs + p, l);
    }
    
    if(halfsize != wav->size/2)
    {
        waveuntrans(wav, buff, halfsize*2, cs);
    }
}


void KisWaveletNoiseReduction::process(KisPaintDeviceImplSP src, KisPaintDeviceImplSP dst, KisFilterConfiguration* config, const QRect& rect)
{
//     KisID idtrans, iduntrans;
/*    if(!KisFilterRegistry::instance()->search("Fast Wavelet Transform", idtrans) || !KisFilterRegistry::instance()->search("Fast Wavelet Untransform", iduntrans))
    {
        kdDebug() << "Unable to find Fast Wavelet Transform or Untransform, check your install" << endl;
        return;
    }
    KisFilterSP ftrans = KisFilterRegistry::instance()->get(idtrans);
    KisFilterSP funtrans = KisFilterRegistry::instance()->get(iduntrans);*/
    
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
    
    kdDebug() << size << " " << maxrectsize << " " << rect.x() << " " << rect.y() << endl;
    Wavelet* wav = new Wavelet(size, depth);
    
    for(int i = rect.y(); i < rect.height(); i++)
    {
        KisHLineIteratorPixel srcIt = src->createHLineIterator(rect.x(), i, rect.width(), false );
        float *dstIt = wav->coeffs + (i-rect.y())*wav->size*wav->depth;
        while( ! srcIt.isDone() )
        {
            Q_UINT8* v1 = srcIt.rawData();
            for( int k = 0; k < depth; k++)
            {
                *dstIt = (float)v1[k];
                ++dstIt;
            }
            ++srcIt;
        }
    }
    Wavelet* buff = new Wavelet(size, depth);
    kdDebug() << "Transforming..." << endl;
    wavetrans(wav, buff, size/2);
    
#if 0
    KisImageSP intermImage = new KisImage(0, wav->size, wav->size, src->colorSpace(), "intermImage.NoiseReduction");
    Q_CHECK_PTR(intermImage);
    KisLayerSP intermLay = new KisLayer(intermImage, intermImage -> nextLayerName(), OPACITY_OPAQUE);
    intermImage -> add(intermLay, -1);

    for(int i = rect.y(); i < rect.height(); i++)
    {
        KisHLineIteratorPixel dstIt = intermLay->createHLineIterator(rect.x(), i, rect.width(), true );
        float *srcIt = wav->coeffs + (i-rect.y())*wav->size*wav->depth;
        while( ! dstIt.isDone() )
        {
            Q_UINT8* v1 = dstIt.rawData();
            for( int k = 0; k < depth; k++)
            {
                v1[k] = (int)*srcIt;
                ++srcIt;
            }
            ++dstIt;
        }
    }
    saveToFile((KisPaintDeviceImplSP)intermLay, "test.il.png");
#endif
    
    kdDebug() << "Thresholding..." << endl;
    float* fin = wav->coeffs + wav->depth*wav->size*wav->size;
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
    }
    
    kdDebug() << "Untransforming..." << endl;
    waveuntrans(wav, buff, 1, src->colorSpace());

    for(int i = rect.y(); i < rect.height(); i++)
    {
        KisHLineIteratorPixel dstIt = dst->createHLineIterator(rect.x(), i, rect.width(), true );
        float *srcIt = wav->coeffs + (i-rect.y())*wav->size*wav->depth;
        while( ! dstIt.isDone() )
        {
            Q_UINT8* v1 = dstIt.rawData();
            for( int k = 0; k < depth; k++)
            {
                v1[k] = (int)*srcIt;
                ++srcIt;
            }
            ++dstIt;
        }
    }
    delete wav;
    delete buff;
    
    
#if 0
//     KisImageSP intermImage = new KisImage(0, size+rect.x(), size+rect.y(), src->colorSpace(), "intermImage.NoiseReduction");
//     Q_CHECK_PTR(intermImage);
//     KisLayerSP intermLay = new KisLayer(intermImage, intermImage -> nextLayerName(), OPACITY_OPAQUE);
//     intermImage -> add(intermLay, -1);
    
    
//     int depth = src -> colorSpace() -> nColorChannels();
    {
        KisRectIteratorPixel copy_dstIt = intermLay->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );
        KisRectIteratorPixel copy_srcIt = src->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), false);
        while( ! copy_srcIt.isDone() )
        {
            for( int i = 0; i < depth; i++)
            {
                copy_dstIt.rawData()[i] = copy_srcIt.rawData()[i];
            }
            ++copy_srcIt;
            ++copy_dstIt;
        }
    }
    
    
    ftrans->process((KisPaintDeviceImplSP)intermLay, (KisPaintDeviceImplSP)intermLay, 0, rect);
    funtrans->process((KisPaintDeviceImplSP)intermLay, (KisPaintDeviceImplSP)intermLay, 0, rect);
    {
        KisRectIteratorPixel copy_dstIt = dst->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );
        KisRectIteratorPixel copy_srcIt = intermLay->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), false);
        while( ! copy_srcIt.isDone() )
        {
            for( int i = 0; i < depth; i++)
            {
                copy_dstIt.rawData()[i] = copy_srcIt.rawData()[i];
            }
            ++copy_srcIt;
            ++copy_dstIt;
        }
    }
#endif
    setProgressDone(); // Must be called even if you don't really support progression
}
