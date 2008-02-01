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

template< typename _TYPE_, quint32 _wavelen_number_ >
struct KisKSColorSpaceTrait : public KoColorSpaceTrait<_TYPE_, 2*_wavelen_number_+1, 2*_wavelen_number_> {

    typedef KoColorSpaceTrait<_TYPE_, 2*(_wavelen_number_)+1, 6> parent;

    struct {
        _TYPE_ m_K;
        _TYPE_ m_S;
    } wavelength[_wavelen_number_];
    _TYPE_ m_opacity;

    inline static _TYPE_ &K(quint8* data, const quint32 wavelen)
    {
        _TYPE_ *d = reinterpret_cast<_TYPE_ *>(data);
        return d[2*wavelen+0];
    }

    inline static _TYPE_ &S(quint8* data, const quint32 wavelen)
    {
        _TYPE_ *d = reinterpret_cast<_TYPE_ *>(data);
        return d[2*wavelen+1];
    }

    inline static _TYPE_ &nativealpha(quint8* data)
    {
        return reinterpret_cast<_TYPE_ *>(data)[2*_wavelen_number_];
    }

    inline static const _TYPE_ &K(const quint8* data, const quint32 wavelen)
    {
        const _TYPE_ *d = reinterpret_cast<const _TYPE_ *>(data);
        return d[2*wavelen+0];
    }

    inline static const _TYPE_ &S(const quint8* data, const quint32 wavelen)
    {
        const _TYPE_ *d = reinterpret_cast<const _TYPE_ *>(data);
        return d[2*wavelen+1];
    }

    inline static const _TYPE_ &nativealpha(const quint8* data)
    {
        return reinterpret_cast<const _TYPE_ *>(data)[2*_wavelen_number_];
    }
};

#endif // KIS_KS_COLORSPACE_TRAITS_H_
