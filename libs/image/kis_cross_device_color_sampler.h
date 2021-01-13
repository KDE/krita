/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_CROSS_DEVICE_COLOR_SAMPLER_H
#define __KIS_CROSS_DEVICE_COLOR_SAMPLER_H

#include "KoColorSpace.h"
#include "kis_random_sub_accessor.h"


struct SamplerTraitReal {
    typedef qreal coord_type;
    typedef KisRandomSubAccessorSP accessor_type;
    static inline accessor_type createAccessor(KisPaintDeviceSP dev) {
        return dev->createRandomSubAccessor();
    }

    template <bool useOldData>
    static inline void sampleData(accessor_type accessor, quint8 *data, const KoColorSpace*) {
        if (useOldData) {
            accessor->sampledOldRawData(data);
        } else {
            accessor->sampledRawData(data);
        }
    }
};

struct SamplerTraitInt {
    typedef int coord_type;
    typedef KisRandomConstAccessorSP accessor_type;
    static inline accessor_type createAccessor(KisPaintDeviceSP dev) {
        return dev->createRandomConstAccessorNG();
    }

    template <bool useOldData>
    static inline void sampleData(accessor_type accessor, quint8 *data, const KoColorSpace *cs) {
        if (useOldData) {
            memcpy(data, accessor->oldRawData(), cs->pixelSize());
        } else {
            memcpy(data, accessor->rawDataConst(), cs->pixelSize());
        }
    }
};

/**
 * The sampler class is supposed to help to sample color from one device
 * and automatically convert it to the color space of another device
 *
 * WARNING: Please note, that if you want to access correct rawData(),
 *          you shouldn't store the sampler class (as well as any
 *          random accessor class) across different calls to
 *          paintAt. This is related to the fact that
 *          KisRandomAccessor has an internal cache of the tiles, but
 *          any tile may become 'old' with the time, so you'll end up
 *          reading from the old tile instead of current one.
 */

template <class Traits>
class KisCrossDeviceColorSamplerImpl
{
public:
    KisCrossDeviceColorSamplerImpl(KisPaintDeviceSP src, KisPaintDeviceSP dst) {
        init(src, dst);
    }

    KisCrossDeviceColorSamplerImpl(KisPaintDeviceSP src, KisFixedPaintDeviceSP dst) {
        init(src, dst);
    }

    KisCrossDeviceColorSamplerImpl(KisPaintDeviceSP src, const KoColor &dst) {
        init(src, &dst);
    }

    ~KisCrossDeviceColorSamplerImpl() {
        delete[] m_data;
    }

    inline void sampleColor(typename Traits::coord_type x,
                          typename Traits::coord_type y,
                          quint8 *dst) {
        sampleColorImpl<false>(x, y, dst);
    }

    inline void sampleOldColor(typename Traits::coord_type x,
                             typename Traits::coord_type y,
                             quint8 *dst) {
        sampleColorImpl<true>(x, y, dst);
    }

private:
    template <typename T>
    inline void init(KisPaintDeviceSP src, T dst) {
        m_srcCS = src->colorSpace();
        m_dstCS = dst->colorSpace();
        m_data = new quint8[m_srcCS->pixelSize()];

        m_accessor = Traits::createAccessor(src);
    }

    template <bool useOldData>
    inline void sampleColorImpl(typename Traits::coord_type x,
                              typename Traits::coord_type y,
                              quint8 *dst) {
        m_accessor->moveTo(x, y);

        Traits::template sampleData<useOldData>(m_accessor, m_data, m_srcCS);

        m_srcCS->convertPixelsTo(m_data, dst, m_dstCS, 1,
                                 KoColorConversionTransformation::internalRenderingIntent(),
                                 KoColorConversionTransformation::internalConversionFlags());
    }

private:
    const KoColorSpace *m_srcCS;
    const KoColorSpace *m_dstCS;
    typename Traits::accessor_type m_accessor;
    quint8 *m_data;
};

typedef KisCrossDeviceColorSamplerImpl<SamplerTraitReal> KisCrossDeviceColorSampler;
typedef KisCrossDeviceColorSamplerImpl<SamplerTraitInt> KisCrossDeviceColorSamplerInt;

#endif /* __KIS_CROSS_DEVICE_COLOR_SAMPLER_H */
