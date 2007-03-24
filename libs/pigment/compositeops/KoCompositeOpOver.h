/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef _KOCOMPOSITEOPOVER_H_
#define _KOCOMPOSITEOPOVER_H_

#include "KoCompositeOpAlphaBase.h"


/**
 * A template version of the over composite operation to use in colorspaces.
 */
template<class _CSTraits>
class KoCompositeOpOver : public KoCompositeOpAlphaBase<_CSTraits, KoCompositeOpOver<_CSTraits> > {
    typedef typename _CSTraits::channels_type channels_type;
    public:

        KoCompositeOpOver(KoColorSpace * cs)
        : KoCompositeOpAlphaBase<_CSTraits, KoCompositeOpOver<_CSTraits> >(cs, COMPOSITE_OVER, i18n("Normal" ) )
        {
        }

    public:

        inline static void composeColorChannels( channels_type srcBlend,
                                                 const channels_type* srcN,
                                                 channels_type* dstN,
                                                 qint32 pixelSize,
                                                 const QBitArray & channelFlags )
        {
            if (srcBlend == NATIVE_OPACITY_OPAQUE) {
                memcpy(dstN, srcN, pixelSize);
            } else {
                for(uint i = 0; i <  _CSTraits::channels_nb; i++)
                {
                    if( (int)i != _CSTraits::alpha_pos && (  channelFlags.isEmpty() || channelFlags.testBit( i ) ) )
                        dstN[i] = KoColorSpaceMaths<channels_type>::blend(srcN[i], dstN[i], srcBlend);
                }
            }
        }

};

#endif
