/*
 *  Copyright (c) 2007 Emanuele Tamponi <emanuele@valinor.it>
 *
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

#ifndef KIS_KS_COLORSPACE_TRAITS_H_
#define KIS_KS_COLORSPACE_TRAITS_H_

#include <KoColorSpaceTraits.h>

template<int _wavelen_number_>
struct KisKSColorSpaceTrait : public KoColorSpaceTrait<float, 2*(_wavelen_number_)+1, 6> {

    typedef KoColorSpaceTrait<float, 2*(_wavelen_number_)+1, 6> parent;

    float m_K[_wavelen_number_];
    float m_S[_wavelen_number_];
    float m_opacity;

    inline static float &K(quint8* data, int wavelen)
    {
        float *d = parent::nativeArray(data);
        // User asked for K that's in the first [0 ... _wavelen_number_-1] positions
        return d[wavelen];
    }

    inline static float &S(quint8* data, int wavelen)
    {
        float *d = parent::nativeArray(data);
        // User asked for S that's in the [ _wavelen_number_ ... 2*_wavelen_number_ - 1] positions
        return d[_wavelen_number_ + wavelen];
    }

    inline static const float &K(const quint8* data, int wavelen)
    {
        const float *d = parent::nativeArray(data);
        // User asked for K that's in the first [0 ... _wavelen_number_-1] positions
        return d[wavelen];
    }

    inline static const float &S(const quint8* data, int wavelen)
    {
        const float *d = parent::nativeArray(data);
        // User asked for S that's in the [ _wavelen_number_ ... 2*_wavelen_number_ - 1] positions
        return d[_wavelen_number_ + wavelen];
    }
};

typedef KisKSColorSpaceTrait<3> KisKS3ColorSpaceTrait;
typedef KisKSColorSpaceTrait<9> KisKS9ColorSpaceTrait;

#endif // KIS_KS_COLORSPACE_TRAITS_H_
