/*
 *  This file is part of the KDE project
 *
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

#include "kis_math_toolbox.h"
#include <config-krita.h>
#ifdef HAVE_OPENEXR
#include <half.h>
#endif

#include "kis_basic_math_toolbox.h"
#include "kis_iterators_pixel.h"
//Added by qt3to4:
#include <Q3MemArray>


KisMathToolbox::KisMathToolbox(KisID id) : m_id(id)
{
}

KisMathToolbox::~KisMathToolbox()
{
}

KisMathToolboxFactoryRegistry::KisMathToolboxFactoryRegistry()
{
    add(new KisBasicMathToolbox());
}
KisMathToolboxFactoryRegistry::~KisMathToolboxFactoryRegistry()
{
}
template<typename T>
double toDouble(quint8* data, int channelpos )
{
    return (float)( *((T*)(data + channelpos)) );
}

typedef double (*PtrToDouble)(quint8*, int);

template<typename T>
void fromDouble(quint8* data, int channelpos, double v )
{
    *((T*)(data + channelpos)) = (T)v;
}

typedef void (*PtrFromDouble)(quint8*, int, double);


void KisMathToolbox::transformToFR(KisPaintDeviceSP src, KisFloatRepresentation* fr, const QRect& rect)
{
    qint32 depth = src->colorSpace()->nColorChannels();
    Q3MemArray<PtrToDouble> f(depth);
    Q3ValueVector<KisChannelInfo *> cis = src->colorSpace()->channels();
    for(qint32 k = 0; k < depth; k++)
    {
        switch( cis[k]->channelValueType() )
        {
            case KisChannelInfo::UINT8:
                f[k] = toDouble<quint8>;
                break;
            case KisChannelInfo::UINT16:
                f[k] = toDouble<quint16>;
                break;
#ifdef HAVE_OPENEXR		
            case KisChannelInfo::FLOAT16:
                f[k] = toDouble<half>;
                break;
#endif
            case KisChannelInfo::FLOAT32:
                f[k] = toDouble<float>;
                break;
            case KisChannelInfo::INT8:
                f[k] = toDouble<qint8>;
                break;
            case KisChannelInfo::INT16:
                f[k] = toDouble<qint16>;
                break;
            default:
                kWarning() << "Unsupported value type in KisMathToolbox" << endl;
                return;
        }
    }
    
    for(int i = rect.y(); i < rect.height(); i++)
    {
        KisHLineIteratorPixel srcIt = src->createHLineIterator(rect.x(), i, rect.width(), false );
        float *dstIt = fr->coeffs + (i-rect.y()) * fr->size * fr->depth;
        while( ! srcIt.isDone() )
        {
            quint8* v1 = srcIt.rawData();
            for( int k = 0; k < depth; k++)
            {
                *dstIt = f[k](v1, cis[k]->pos());
                ++dstIt;
            }
            ++srcIt;
        }
    }
}

void KisMathToolbox::transformFromFR(KisPaintDeviceSP dst, KisFloatRepresentation* fr, const QRect& rect)
{
    qint32 depth = dst->colorSpace()->nColorChannels();
    Q3MemArray<PtrFromDouble> f(depth);
    Q3ValueVector<KisChannelInfo *> cis = dst->colorSpace()->channels();
    for(qint32 k = 0; k < depth; k++)
    {
        switch( cis[k]->channelValueType() )
        {
            case KisChannelInfo::UINT8:
                f[k] = fromDouble<quint8>;
                break;
            case KisChannelInfo::UINT16:
                f[k] = fromDouble<quint16>;
                break;
#ifdef HAVE_OPENEXR
            case KisChannelInfo::FLOAT16:
                f[k] = fromDouble<half>;
                break;
#endif		
            case KisChannelInfo::FLOAT32:
                f[k] = fromDouble<float>;
                break;
            case KisChannelInfo::INT8:
                f[k] = fromDouble<qint8>;
                break;
            case KisChannelInfo::INT16:
                f[k] = fromDouble<qint16>;
                break;
            default:
                kWarning() << "Unsupported value type in KisMathToolbox" << endl;
                return;
        }
    }
    for(int i = rect.y(); i < rect.height(); i++)
    {
        KisHLineIteratorPixel dstIt = dst->createHLineIterator(rect.x(), i, rect.width(), true );
        float *srcIt = fr->coeffs + (i-rect.y()) * fr->size * fr->depth;
        while( ! dstIt.isDone() )
        {
            quint8* v1 = dstIt.rawData();
            for( int k = 0; k < depth; k++)
            {
                f[k](v1, cis[k]->pos(), *srcIt);
                ++srcIt;
            }
            ++dstIt;
        }
    }
}

#include "kis_math_toolbox.moc"
