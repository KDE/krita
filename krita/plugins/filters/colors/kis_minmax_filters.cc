/*
 * This file is part of Krita
 *
 * Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include "kis_minmax_filters.h"

#include <kis_iterators_pixel.h>

typedef void (*funcMaxMin)(const Q_UINT8* , Q_UINT8* , uint );

template<typename _TYPE>
void maximize(const Q_UINT8* s, Q_UINT8* d, uint nbpixels)
{
    const _TYPE* sT = (_TYPE*)(s);
    _TYPE* dT = (_TYPE*)(d);
    _TYPE vmax = *sT;
    for(uint i = 1; i < nbpixels; i ++)
    {
        if(sT[i] > vmax)
        {
            vmax = sT[i];
        }
    }
    for(uint i = 0; i < nbpixels; i ++)
    {
        if(dT[i] != vmax)
        {
            dT[i] = 0;
        }
    }
}

template<typename _TYPE>
        void minimize(const Q_UINT8* s, Q_UINT8* d, uint nbpixels)
{
    const _TYPE* sT = (_TYPE*)(s);
    _TYPE* dT = (_TYPE*)(d);
    _TYPE vmin = *sT;
    for(uint i = 1; i < nbpixels; i ++)
    {
        if(sT[i] < vmin)
        {
            vmin = sT[i];
        }
    }
    for(uint i = 0; i < nbpixels; i ++)
    {
        if(dT[i] != vmin)
        {
            dT[i] = 0;
        }
    }
}

KisFilterMax::KisFilterMax() : KisFilter(id(), "colors", i18n("M&aximize channel"))
{
}

void KisFilterMax::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* /*config*/, const QRect& rect)
{
    Q_ASSERT(src != 0);
    Q_ASSERT(dst != 0);
    
    KisRectIteratorPixel dstIt = dst->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );
    KisRectIteratorPixel srcIt = src->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), false);

    int pixelsProcessed = 0;
    setProgressTotalSteps(rect.width() * rect.height());

    KisColorSpace * cs = src->colorSpace();
    Q_INT32 nC = cs->nColorChannels();
    
    funcMaxMin F;
    KisChannelInfo::enumChannelValueType cT = cs->channels()[0]->channelValueType();
    if( cT == KisChannelInfo::UINT8 || cT == KisChannelInfo::INT8 )
    {
        F = & maximize<Q_UINT8>;
    } else if( cT == KisChannelInfo::UINT16 || cT == KisChannelInfo::INT16 )
    {
        F = & maximize<Q_UINT8>;
    } else if( cT == KisChannelInfo::FLOAT32 )
    {
        F = & maximize<float>;
    } else {
        return;
    }
    
    while( ! srcIt.isDone() )
    {
        if(srcIt.isSelected())
        {
            F( srcIt.oldRawData(), dstIt.rawData(), nC);
        }
        setProgress(++pixelsProcessed);
        ++srcIt;
        ++dstIt;
    }
    setProgressDone(); // Must be called even if you don't really support progression
}

KisFilterMin::KisFilterMin() : KisFilter(id(), "colors", i18n("M&inimize channel"))
{
}

void KisFilterMin::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* /*config*/, const QRect& rect)
{
    Q_ASSERT(src != 0);
    Q_ASSERT(dst != 0);
    
    KisRectIteratorPixel dstIt = dst->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );
    KisRectIteratorPixel srcIt = src->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), false);

    int pixelsProcessed = 0;
    setProgressTotalSteps(rect.width() * rect.height());

    KisColorSpace * cs = src->colorSpace();
    Q_INT32 nC = cs->nColorChannels();
    
    funcMaxMin F;
    KisChannelInfo::enumChannelValueType cT = cs->channels()[0]->channelValueType();
    if( cT == KisChannelInfo::UINT8 || cT == KisChannelInfo::INT8 )
    {
        F = & minimize<Q_UINT8>;
    } else if( cT == KisChannelInfo::UINT16 || cT == KisChannelInfo::INT16 )
    {
        F = & minimize<Q_UINT8>;
    } else if( cT == KisChannelInfo::FLOAT32 )
    {
        F = & minimize<float>;
    } else {
        return;
    }
    
    while( ! srcIt.isDone() )
    {
        if(srcIt.isSelected())
        {
            F( srcIt.oldRawData(), dstIt.rawData(), nC);
        }
        setProgress(++pixelsProcessed);
        ++srcIt;
        ++dstIt;
    }
    setProgressDone(); // Must be called even if you don't really support progression
}

