/*
 *  This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_math_toolbox.h"

#include <KoConfig.h>

#ifdef HAVE_OPENEXR
#include <half.h>
#endif

#include <QVector>
#include <QGlobalStatic>

#include <KoColorSpaceMaths.h>

#include <kis_debug.h>
#include "kis_iterator_ng.h"

#include "math.h"

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

template<typename T>
void fromDoubleCheckNull(quint8* data, int channelpos, double v, bool *isNull)
{
    T value = qRound(v);
    *((T*)(data + channelpos)) = value;
    *isNull = value == T(0);
}

template<typename T>
void fromDoubleCheckNullF(quint8* data, int channelpos, double v, bool *isNull)
{
    T value = v;
    *((T*)(data + channelpos)) = (T)v;
    *isNull = value < std::numeric_limits<T>::epsilon();
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

bool KisMathToolbox::getFromDoubleCheckNullChannelPtr(QList<KoChannelInfo *> cis, QVector<PtrFromDoubleCheckNull>& f)
{
    qint32 channels = cis.count();

    for (qint32 k = 0; k < channels; k++) {
        switch (cis[k]->channelValueType()) {
        case KoChannelInfo::UINT8:
            f[k] = fromDoubleCheckNull<quint8>;
            break;
        case KoChannelInfo::UINT16:
            f[k] = fromDoubleCheckNull<quint16>;
            break;
#ifdef HAVE_OPENEXR
        case KoChannelInfo::FLOAT16:
            f[k] = fromDoubleCheckNullF<half>;
            break;
#endif
        case KoChannelInfo::FLOAT32:
            f[k] = fromDoubleCheckNullF<float>;
            break;
        case KoChannelInfo::INT8:
            f[k] = fromDoubleCheckNull<qint8>;
            break;
        case KoChannelInfo::INT16:
            f[k] = fromDoubleCheckNull<qint16>;
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

void KisMathToolbox::wavetrans(KisMathToolbox::KisWavelet* wav, KisMathToolbox::KisWavelet* buff, uint halfsize)
{
    uint l = (2 * halfsize) * wav->depth * sizeof(float);
    for (uint i = 0; i < halfsize; i++) {
        float * itLL = buff->coeffs + i * buff->size * buff->depth;
        float * itHL = buff->coeffs + (i * buff->size + halfsize) * buff->depth;
        float * itLH = buff->coeffs + (halfsize + i) * buff->size * buff->depth;
        float * itHH = buff->coeffs + ((halfsize + i) * buff->size + halfsize) * buff->depth;
        float * itS11 = wav->coeffs + 2 * i * wav->size * wav->depth;
        float * itS12 = wav->coeffs + (2 * i * wav->size + 1) * wav->depth;
        float * itS21 = wav->coeffs + (2 * i + 1) * wav->size * wav->depth;
        float * itS22 = wav->coeffs + ((2 * i + 1) * wav->size + 1) * wav->depth;
        for (uint j = 0; j < halfsize; j++) {
            for (uint k = 0; k < wav->depth; k++) {
                *(itLL++) = (*itS11 + *itS12 + *itS21 + *itS22) * M_SQRT1_2;
                *(itHL++) = (*itS11 - *itS12 + *itS21 - *itS22) * M_SQRT1_2;
                *(itLH++) = (*itS11 + *itS12 - *itS21 - *itS22) * M_SQRT1_2;
                *(itHH++) = (*(itS11++) - *(itS12++) - *(itS21++) + *(itS22++)) * M_SQRT1_2;
            }
            itS11 += wav->depth; itS12 += wav->depth;
            itS21 += wav->depth; itS22 += wav->depth;
        }
    }
    for (uint i = 0; i < halfsize; i++) {
        uint p = i * wav->size * wav->depth;
        memcpy(wav->coeffs + p, buff->coeffs + p, l);
        p = (i + halfsize) * wav->size * wav->depth;
        memcpy(wav->coeffs + p, buff->coeffs + p, l);
    }
    if (halfsize != 1) {
        wavetrans(wav, buff, halfsize / 2);
    }
}

void KisMathToolbox::waveuntrans(KisMathToolbox::KisWavelet* wav, KisMathToolbox::KisWavelet* buff, uint halfsize)
{
    uint l = (2 * halfsize) * wav->depth * sizeof(float);
    for (uint i = 0; i < halfsize; i++) {
        float * itLL = wav->coeffs + i * buff->size * buff->depth;
        float * itHL = wav->coeffs + (i * buff->size + halfsize) * buff->depth;
        float * itLH = wav->coeffs + (halfsize + i) * buff->size * buff->depth;
        float * itHH = wav->coeffs + ((halfsize + i) * buff->size + halfsize) * buff->depth;
        float * itS11 = buff->coeffs + 2 * i * wav->size * wav->depth;
        float * itS12 = buff->coeffs + (2 * i * wav->size + 1) * wav->depth;
        float * itS21 = buff->coeffs + (2 * i + 1) * wav->size * wav->depth;
        float * itS22 = buff->coeffs + ((2 * i + 1) * wav->size + 1) * wav->depth;
        for (uint j = 0; j < halfsize; j++) {
            for (uint k = 0; k < wav->depth; k++) {
                *(itS11++) = (*itLL + *itHL + *itLH + *itHH) * 0.25 * M_SQRT2;
                *(itS12++) = (*itLL - *itHL + *itLH - *itHH) * 0.25 * M_SQRT2;
                *(itS21++) = (*itLL + *itHL - *itLH - *itHH) * 0.25 * M_SQRT2;
                *(itS22++) = (*(itLL++) - *(itHL++) - *(itLH++) + *(itHH++)) * 0.25 * M_SQRT2;
            }
            itS11 += wav->depth; itS12 += wav->depth;
            itS21 += wav->depth; itS22 += wav->depth;
        }
    }
    for (uint i = 0; i < halfsize; i++) {
        uint p = i * wav->size * wav->depth;
        memcpy(wav->coeffs + p, buff->coeffs + p, l);
        p = (i + halfsize) * wav->size * wav->depth;
        memcpy(wav->coeffs + p, buff->coeffs + p, l);
    }

    if (halfsize != wav->size / 2) {
        waveuntrans(wav, buff, halfsize*2);
    }
}

KisMathToolbox::KisWavelet* KisMathToolbox::fastWaveletTransformation(KisPaintDeviceSP src, const QRect& rect,  KisWavelet* buff)
{
    if (buff == 0) {
        buff = initWavelet(src, rect);
    }
    KisWavelet* wav = initWavelet(src, rect);
    transformToFR(src, wav, rect);
    wavetrans(wav, buff, wav->size / 2);

    return wav;
}

void KisMathToolbox::fastWaveletUntransformation(KisPaintDeviceSP dst, const QRect& rect, KisWavelet* wav, KisWavelet* buff)
{
    if (buff == 0) {
        buff = initWavelet(dst, rect);
    }

    waveuntrans(wav, buff, 1);
    transformFromFR(dst, wav, rect);
}
