/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_CROSS_DEVICE_COLOR_PICKER_H
#define __KIS_CROSS_DEVICE_COLOR_PICKER_H

#include "kis_paint_device.h"
#include "kis_fixed_paint_device.h"
#include "kis_random_sub_accessor.h"


struct PickerTraitReal {
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

struct PickerTraitInt {
    typedef int coord_type;
    typedef KisRandomAccessorSP accessor_type;
    static inline accessor_type createAccessor(KisPaintDeviceSP dev) {
        return dev->createRandomAccessorNG(0,0);
    }

    template <bool useOldData>
    static inline void sampleData(accessor_type accessor, quint8 *data, const KoColorSpace *cs) {
        if (useOldData) {
            memcpy(data, accessor->oldRawData(), cs->pixelSize());
        } else {
            // FIXME: do not create non-const accessor for non-old data
            memcpy(data, accessor->rawData(), cs->pixelSize());
        }
    }
};

template <class Traits>
class KisCrossDeviceColorPickerImpl
{
public:
    KisCrossDeviceColorPickerImpl(KisPaintDeviceSP src, KisPaintDeviceSP dst) {
        init(src, dst);
    }

    KisCrossDeviceColorPickerImpl(KisPaintDeviceSP src, KisFixedPaintDeviceSP dst) {
        init(src, dst);
    }

    KisCrossDeviceColorPickerImpl(KisPaintDeviceSP src, const KoColor &dst) {
        init(src, &dst);
    }

    ~KisCrossDeviceColorPickerImpl() {
        delete[] m_data;
    }

    inline void pickColor(typename Traits::coord_type x,
                          typename Traits::coord_type y,
                          quint8 *dst) {
        pickColorImpl<false>(x, y, dst);
    }

    inline void pickOldColor(typename Traits::coord_type x,
                             typename Traits::coord_type y,
                             quint8 *dst) {
        pickColorImpl<true>(x, y, dst);
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
    inline void pickColorImpl(typename Traits::coord_type x,
                              typename Traits::coord_type y,
                              quint8 *dst) {
        m_accessor->moveTo(x, y);

        Traits::template sampleData<useOldData>(m_accessor, m_data, m_srcCS);

        m_srcCS->convertPixelsTo(m_data, dst, m_dstCS, 1,
                               KoColorConversionTransformation::IntentPerceptual,
                               KoColorConversionTransformation::BlackpointCompensation);
    }

private:
    const KoColorSpace *m_srcCS;
    const KoColorSpace *m_dstCS;
    typename Traits::accessor_type m_accessor;
    quint8 *m_data;
};

typedef KisCrossDeviceColorPickerImpl<PickerTraitReal> KisCrossDeviceColorPicker;
typedef KisCrossDeviceColorPickerImpl<PickerTraitInt> KisCrossDeviceColorPickerInt;

#endif /* __KIS_CROSS_DEVICE_COLOR_PICKER_H */
