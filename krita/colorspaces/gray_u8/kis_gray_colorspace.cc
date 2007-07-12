/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include <kdebug.h>
#include <klocale.h>

#include <KoIntegerMaths.h>
#include <KoColorSpaceRegistry.h>

#include "kis_gray_colorspace.h"

#include "compositeops/KoCompositeOpErase.h"


namespace {
    const quint8 PIXEL_GRAY = 0;


    class CompositeOver : public KoCompositeOp {

    public:

        CompositeOver(KoColorSpace * cs)
            : KoCompositeOp(cs, COMPOSITE_OVER, i18n("Normal" ) )
        {
        }

    public:

        void composite(quint8 *dst,
                       qint32 dststride,
                       const quint8 *src,
                       qint32 srcstride,
                       const quint8 *maskRowStart,
                       qint32 maskstride,
                       qint32 rows,
                       qint32 cols,
                       quint8 opacity,
                       const QBitArray & channelFlags) const
        {
            Q_UNUSED(channelFlags);

            quint8 *d;
            const quint8 *s;
            qint32 i;


            if (rows <= 0 || cols <= 0)
                return;
            if (opacity == OPACITY_TRANSPARENT)
                return;
            if (opacity != OPACITY_OPAQUE) {
                while (rows-- > 0) {

                    const quint8 *mask = maskRowStart;

                    d = dst;
                    s = src;
                    for (i = cols; i > 0; i--, d++, s++) {
                        // If the mask tells us to completely not
                        // blend this pixel, continue.
                        if ( mask != 0 ) {
                            if ( mask[0] == OPACITY_TRANSPARENT ) {
                                mask++;
                                continue;
                            }
                            mask++;
                        }
                        if (s[PIXEL_GRAY] == OPACITY_TRANSPARENT)
                            continue;
                        int srcGray = (s[PIXEL_GRAY] * opacity + UINT8_MAX / 2) / UINT8_MAX;
                        d[PIXEL_GRAY] = (d[PIXEL_GRAY] * (UINT8_MAX - srcGray) + srcGray * UINT8_MAX + UINT8_MAX / 2) / UINT8_MAX;
                    }
                    dst += dststride;
                    src += srcstride;
                    if(maskRowStart) {
                        maskRowStart += maskstride;
                }
                }
            }
            else {
                while (rows-- > 0) {
                    const quint8 *mask = maskRowStart;

                    d = dst;
                    s = src;
                    for (i = cols; i > 0; i--, d++, s++) {

                        if ( mask != 0 ) {
                            // If the mask tells us to completely not
                            // blend this pixel, continue.
                            if ( mask[0] == OPACITY_TRANSPARENT ) {
                                mask++;
                                continue;
                            }
                            mask++;
                        }

                        if (s[PIXEL_GRAY] == OPACITY_TRANSPARENT)
                            continue;
                        if (d[PIXEL_GRAY] == OPACITY_TRANSPARENT || s[PIXEL_GRAY] == OPACITY_OPAQUE) {
                            memcpy(d, s, 1);
                            continue;
                        }
                        int srcGray = s[PIXEL_GRAY];
                        d[PIXEL_GRAY] = (d[PIXEL_GRAY] * (UINT8_MAX - srcGray) + srcGray * UINT8_MAX + UINT8_MAX / 2) / UINT8_MAX;
                    }
                    dst += dststride;
                    src += srcstride;
                    if(maskRowStart) {
                        maskRowStart += maskstride;
                }
                }
            }
        }
    };

}

KisGrayColorSpace ::KisGrayColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p) :
            KoLcmsColorSpace<GrayU8Traits>("GRAYU8", i18n("Grayscale 8-bit integer/channel)"), parent, TYPE_GRAY_8, icSigGrayData, p)
{
    addChannel(new KoChannelInfo(i18n("Gray"), 0, KoChannelInfo::COLOR, KoChannelInfo::UINT8));

    init();

    addCompositeOp( new CompositeOver( this ) );
    addCompositeOp( new KoCompositeOpErase<GrayU8Traits>( this ) );
}
