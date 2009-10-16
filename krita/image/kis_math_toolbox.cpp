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

#include <config-openexr.h>

#ifdef HAVE_OPENEXR
#include <half.h>
#endif

#include <QVector>

#include <kglobal.h>

#include <kis_debug.h>

#include "kis_basic_math_toolbox.h"
#include "kis_iterators_pixel.h"


KisMathToolbox::KisMathToolbox(KoID id) : m_id(id)
{
}

KisMathToolbox::~KisMathToolbox()
{
}

KisMathToolboxRegistry * KisMathToolboxRegistry::instance()
{
    K_GLOBAL_STATIC(KisMathToolboxRegistry, s_instance);
    return s_instance;
}

KisMathToolboxRegistry::KisMathToolboxRegistry()
{
    add(new KisBasicMathToolbox());
}

KisMathToolboxRegistry::~KisMathToolboxRegistry()
{
    dbgRegistry << "deleting KisMathToolboxRegistry";
}

template<typename T>
double toDouble(const quint8* data, int channelpos)
{
    return (float)(*((T*)(data + channelpos)));
}

typedef double(*PtrToDouble)(const quint8*, int);

template<typename T>
void fromDouble(quint8* data, int channelpos, double v)
{
    *((T*)(data + channelpos)) = (T)v;
}

typedef void (*PtrFromDouble)(quint8*, int, double);

void KisMathToolbox::transformToFR(KisPaintDeviceSP src, KisFloatRepresentation* fr, const QRect& rect)
{
    qint32 depth = src->colorSpace()->colorChannelCount();
    QVector<PtrToDouble> f(depth);
    QList<KoChannelInfo *> cis = src->colorSpace()->channels();
    for (qint32 k = 0; k < depth; k++) {
        switch (cis[k]->channelValueType()) {
        case KoChannelInfo::UINT8:
            f[k] = toDouble<quint8>;
            break;
        case KoChannelInfo::UINT16:
            f[k] = toDouble<quint16>;
            break;
#ifdef HAVE_OPENEXR
        case KoChannelInfo::FLOAT16:
            f[k] = toDouble<half>;
            break;
#endif
        case KoChannelInfo::FLOAT32:
            f[k] = toDouble<float>;
            break;
        case KoChannelInfo::INT8:
            f[k] = toDouble<qint8>;
            break;
        case KoChannelInfo::INT16:
            f[k] = toDouble<qint16>;
            break;
        default:
            warnKrita << "Unsupported value type in KisMathToolbox";
            return;
        }
    }

    KisHLineConstIteratorPixel srcIt = src->createHLineIterator(rect.x(), rect.y(), rect.width());

    for (int i = rect.y(); i < rect.height(); i++) {
        float *dstIt = fr->coeffs + (i - rect.y()) * fr->size * fr->depth;
        while (! srcIt.isDone()) {
            const quint8* v1 = srcIt.rawData();
            for (int k = 0; k < depth; k++) {
                *dstIt = f[k](v1, cis[k]->pos());
                ++dstIt;
            }
            ++srcIt;
        }
        srcIt.nextRow();
    }
}

void KisMathToolbox::transformFromFR(KisPaintDeviceSP dst, KisFloatRepresentation* fr, const QRect& rect)
{
    qint32 depth = dst->colorSpace()->colorChannelCount();
    QVector<PtrFromDouble> f(depth);
    QList<KoChannelInfo *> cis = dst->colorSpace()->channels();
    for (qint32 k = 0; k < depth; k++) {
        switch (cis[k]->channelValueType()) {
        case KoChannelInfo::UINT8:
            f[k] = fromDouble<quint8>;
            break;
        case KoChannelInfo::UINT16:
            f[k] = fromDouble<quint16>;
            break;
#ifdef HAVE_OPENEXR
        case KoChannelInfo::FLOAT16:
            f[k] = fromDouble<half>;
            break;
#endif
        case KoChannelInfo::FLOAT32:
            f[k] = fromDouble<float>;
            break;
        case KoChannelInfo::INT8:
            f[k] = fromDouble<qint8>;
            break;
        case KoChannelInfo::INT16:
            f[k] = fromDouble<qint16>;
            break;
        default:
            warnKrita << "Unsupported value type in KisMathToolbox";
            return;
        }
    }

    KisHLineIteratorPixel dstIt = dst->createHLineIterator(rect.x(), rect.y(), rect.width());
    for (int i = rect.y(); i < rect.height(); i++) {
        float *srcIt = fr->coeffs + (i - rect.y()) * fr->size * fr->depth;
        while (! dstIt.isDone()) {
            quint8* v1 = dstIt.rawData();
            for (int k = 0; k < depth; k++) {
                f[k](v1, cis[k]->pos(), *srcIt);
                ++srcIt;
            }
            ++dstIt;
        }
        dstIt.nextRow();
    }
}

#include "kis_math_toolbox.moc"
