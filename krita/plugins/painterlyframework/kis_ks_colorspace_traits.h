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

template< typename TYPE, quint32 _wavelen_number_ >
struct KisKSColorSpaceTrait : public KoColorSpaceTrait<TYPE, 2*_wavelen_number_+1, 2*_wavelen_number_> {

    typedef KoColorSpaceTrait<TYPE, 2*(_wavelen_number_)+1, 6> parent;

    struct {
        TYPE m_K;
        TYPE m_S;
    } wavelenght[_wavelen_number_];
    TYPE m_opacity;

    inline static float &K(quint8* data, const quint32 wavelen)
    {
        float *d = reinterpret_cast<float *>(data);
        return d[2*wavelen+0];
    }

    inline static float &S(quint8* data, const quint32 wavelen)
    {
        float *d = reinterpret_cast<float *>(data);
        return d[2*wavelen+1];
    }

    inline static const float &K(const quint8* data, const quint32 wavelen)
    {
        const float *d = reinterpret_cast<const float *>(data);
        return d[2*wavelen+0];
    }

    inline static const float &S(const quint8* data, const quint32 wavelen)
    {
        const float *d = reinterpret_cast<const float *>(data);
        return d[2*wavelen+1];
    }
};

#endif // KIS_KS_COLORSPACE_TRAITS_H_
