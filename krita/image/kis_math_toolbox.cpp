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

#include <KoConfig.h>

#ifdef HAVE_OPENEXR
#include <half.h>
#endif

#include <QVector>

#include <KoColorSpaceMaths.h>
#include <kglobal.h>

#include <kis_debug.h>

#include "kis_basic_math_toolbox.h"
#include "kis_iterator_ng.h"

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
    foreach(QString id, keys()) {
        delete get(id);
    }
    dbgRegistry << "Deleting KisMathToolboxRegistry";
}

template<typename T>
inline double toDouble(const quint8* data, int channelpos)
{
    return (double)(*((T*)(data + channelpos)));
}

template<typename T>
void fromDouble(quint8* data, int channelpos, double v)
{
    *((T*)(data + channelpos)) = (T)qRound(v);
}

template<typename T>
void fromDoubleF(quint8* data, int channelpos, double v)
{
    *((T*)(data + channelpos)) = (T)v;
}

void KisMathToolbox::transformToFR(KisPaintDeviceSP src, KisFloatRepresentation* fr, const QRect& rect)
{
    qint32 depth = src->colorSpace()->colorChannelCount();
    QList<KoChannelInfo *> cis = src->colorSpace()->channels();
    // remove non-color channels
    for (qint32 c = 0; c < cis.count(); ++c) {
        if (cis[c]->channelType() != KoChannelInfo::COLOR)
            cis.removeAt(c--);
    }
    QVector<PtrToDouble> f(depth);
    if (!getToDoubleChannelPtr(cis, f))
        return;

    KisHLineConstIteratorSP srcIt = src->createHLineIteratorNG(rect.x(), rect.y(), rect.width());

    for (int i = rect.y(); i < rect.height(); i++) {
        float *dstIt = fr->coeffs + (i - rect.y()) * fr->size * fr->depth;
        do {
            const quint8* v1 = srcIt->oldRawData();
            for (int k = 0; k < depth; k++) {
                *dstIt = f[k](v1, cis[k]->pos());
                ++dstIt;
            }
        } while (srcIt->nextPixel());
        srcIt->nextRow();
    }
}

bool KisMathToolbox::getToDoubleChannelPtr(QList<KoChannelInfo *> cis, QVector<PtrToDouble>& f)
{
    qint32 channels = cis.count();

    for (qint32 k = 0; k < channels; k++) {
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
            return false;
        }
    }

    return true;
}

void KisMathToolbox::transformFromFR(KisPaintDeviceSP dst, KisFloatRepresentation* fr, const QRect& rect)
{
    qint32 depth = dst->colorSpace()->colorChannelCount();
    QList<KoChannelInfo *> cis = dst->colorSpace()->channels();
    // remove non-color channels
    for (qint32 c = 0; c < cis.count(); ++c) {
        if (cis[c]->channelType() != KoChannelInfo::COLOR)
            cis.removeAt(c--);
    }

    QVector<PtrFromDouble> f(depth);
    if (!getFromDoubleChannelPtr(cis, f))
        return;

    KisHLineIteratorSP dstIt = dst->createHLineIteratorNG(rect.x(), rect.y(), rect.width());
    for (int i = rect.y(); i < rect.height(); i++) {
        float *srcIt = fr->coeffs + (i - rect.y()) * fr->size * fr->depth;
        do {
            quint8* v1 = dstIt->rawData();
            for (int k = 0; k < depth; k++) {
                f[k](v1, cis[k]->pos(), *srcIt);
                ++srcIt;
            }
        } while(dstIt->nextPixel());
        dstIt->nextRow();
    }
}

bool KisMathToolbox::getFromDoubleChannelPtr(QList<KoChannelInfo *> cis, QVector<PtrFromDouble>& f)
{
    qint32 channels = cis.count();

    for (qint32 k = 0; k < channels; k++) {
        switch (cis[k]->channelValueType()) {
        case KoChannelInfo::UINT8:
            f[k] = fromDouble<quint8>;
            break;
        case KoChannelInfo::UINT16:
            f[k] = fromDouble<quint16>;
            break;
#ifdef HAVE_OPENEXR
        case KoChannelInfo::FLOAT16:
            f[k] = fromDoubleF<half>;
            break;
#endif
        case KoChannelInfo::FLOAT32:
            f[k] = fromDoubleF<float>;
            break;
        case KoChannelInfo::INT8:
            f[k] = fromDouble<qint8>;
            break;
        case KoChannelInfo::INT16:
            f[k] = fromDouble<qint16>;
            break;
        default:
            warnKrita << "Unsupported value type in KisMathToolbox";
            return false;
        }
    }

    return true;
}

double KisMathToolbox::minChannelValue(KoChannelInfo *c)
{
    switch (c->channelValueType())
    {
    case KoChannelInfo::UINT8 : return KoColorSpaceMathsTraits<quint8>::min;
    case KoChannelInfo::UINT16 : return KoColorSpaceMathsTraits<quint16>::min;
    case KoChannelInfo::UINT32 : return KoColorSpaceMathsTraits<quint32>::min;
    #ifdef HAVE_OPENEXR
    case KoChannelInfo::FLOAT16 : return KoColorSpaceMathsTraits<half>::min;
    #endif
    case KoChannelInfo::FLOAT32 : return KoColorSpaceMathsTraits<float>::min;
    case KoChannelInfo::FLOAT64 : return KoColorSpaceMathsTraits<double>::min;
    case KoChannelInfo::INT8 : return 127;
    case KoChannelInfo::INT16 : return KoColorSpaceMathsTraits<qint16>::min;
    default: return 0;
    }
}

double KisMathToolbox::maxChannelValue(KoChannelInfo *c)
{
    switch (c->channelValueType())
    {
    case KoChannelInfo::UINT8 : return KoColorSpaceMathsTraits<quint8>::max;
    case KoChannelInfo::UINT16 : return KoColorSpaceMathsTraits<quint16>::max;
    case KoChannelInfo::UINT32 : return KoColorSpaceMathsTraits<quint32>::max;
    #ifdef HAVE_OPENEXR
    case KoChannelInfo::FLOAT16 : return KoColorSpaceMathsTraits<half>::max;
    #endif
    case KoChannelInfo::FLOAT32 : return KoColorSpaceMathsTraits<float>::max;
    case KoChannelInfo::FLOAT64 : return KoColorSpaceMathsTraits<double>::max;
    case KoChannelInfo::INT8 : return -128;
    case KoChannelInfo::INT16 : return KoColorSpaceMathsTraits<qint16>::max;
    default: return 0;
    }
}

#include "kis_math_toolbox.moc"
