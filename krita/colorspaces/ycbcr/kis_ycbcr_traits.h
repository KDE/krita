/*
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_YCBCR_TRAITS_H_
#define _KIS_YCBCR_TRAITS_H_

#include "KoColorSpaceTraits.h"

const double LUMA_RED = 0.2989;
const double LUMA_GREEN = 0.587;
const double LUMA_BLUE = 0.114;
  

template<typename _channels_type_>
struct KoYCbCrTraits : public KoColorSpaceTrait<_channels_type_, 4, 3> {
    
#define CLAMP_TO_CHANNELSIZE(a) CLAMP(a, 0, KoColorSpaceMathsTraits<_channels_type_>::max)
    static inline _channels_type_ computeRed(_channels_type_ Y, _channels_type_ /*Cb*/, _channels_type_ Cr)
    {
        return (_channels_type_)( CLAMP_TO_CHANNELSIZE( (Cr - MIDDLE_VALUE)* (2-2*LUMA_RED) + Y )  );
    }
    static inline _channels_type_ computeGreen(_channels_type_ Y, _channels_type_ Cb, _channels_type_ Cr)
    {
        return (_channels_type_)( CLAMP_TO_CHANNELSIZE( (Y - LUMA_BLUE * computeBlue(Y,Cb,Cr) - LUMA_RED * computeRed(Y,Cb,Cr) ) / LUMA_GREEN ) );
    }
    static inline _channels_type_ computeBlue(_channels_type_ Y, _channels_type_ Cb, _channels_type_ /*Cr*/)
    {
        return (_channels_type_)( CLAMP_TO_CHANNELSIZE( (Cb - MIDDLE_VALUE)*(2 - 2 * LUMA_BLUE) + Y) );
    }
    static inline _channels_type_ computeY( _channels_type_ r, _channels_type_ b, _channels_type_ g)
    {
        return (_channels_type_)( CLAMP_TO_CHANNELSIZE( LUMA_RED*r + LUMA_GREEN*g + LUMA_BLUE*b ) );
    }
    static inline _channels_type_ computeCb( _channels_type_ r, _channels_type_ b, _channels_type_ g)
    {
        return (_channels_type_)( CLAMP_TO_CHANNELSIZE( (b - computeY(r,g,b))/(2-2*LUMA_BLUE) + MIDDLE_VALUE) );
    }
    static inline _channels_type_ computeCr( _channels_type_ r, _channels_type_ b, _channels_type_ g)
    {
        return (_channels_type_)( CLAMP_TO_CHANNELSIZE( (r - computeY(r,g,b))/(2-2*LUMA_RED) + MIDDLE_VALUE) );
    }
#undef CLAMP_TO_CHANNELSIZE

    static const _channels_type_ MIDDLE_VALUE = (KoColorSpaceMathsTraits<_channels_type_>::max + 1) / 2;
    static const quint8 y_pos = 0;
    static const quint8 cb_pos = 1;
    static const quint8 cr_pos = 2;
    static const quint8 alpha_pos = 3;
    
    struct Pixel {
        _channels_type_ Y;
        _channels_type_ Cb;
        _channels_type_ Cr;
        _channels_type_ alpha;
    };

};


#endif
