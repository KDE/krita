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
#ifndef _KIS_YCBCR_BASE_COLORSPACE_H_
#define _KIS_YCBCR_BASE_COLORSPACE_H_
#include "config-krita.h"

#include "klocale.h"
#include <KoIncompleteColorSpace.h>
#include <KoColorSpaceTraits.h>
#include <KoFallBack.h>

#define UINT8_TO_NATIVE(v) (KoColorSpaceMaths<quint8, typename _CSTraits::channels_type >::scaleToA(v))
#define NATIVE_TO_UINT8(v) (KoColorSpaceMaths<typename _CSTraits::channels_type, quint8>::scaleToA(v))
#define UINT16_TO_NATIVE(v) (KoColorSpaceMaths<quint16, typename _CSTraits::channels_type >::scaleToA(v))
#define NATIVE_TO_UINT16(v) (KoColorSpaceMaths<typename _CSTraits::channels_type, quint16>::scaleToA(v))


template <class _CSTraits>
class KisYCbCrBaseColorSpace : public KoIncompleteColorSpace<_CSTraits, KoRGB16Fallback>
{
    public:
        KisYCbCrBaseColorSpace(const QString &id, const QString &name, KoColorSpaceRegistry * parent, qint32 type)
          : KoIncompleteColorSpace<_CSTraits, KoRGB16Fallback>(id, name, parent, type, icSigRgbData)
        {

        }

        virtual void fromQColor(const QColor& c, quint8 *dstU8, KoColorProfile * /*profile*/) const
        {
            typename _CSTraits::Pixel *dst = reinterpret_cast< typename _CSTraits::Pixel *>(dstU8);
            typename _CSTraits::channels_type red = UINT8_TO_NATIVE(c.red());
            typename _CSTraits::channels_type green = UINT8_TO_NATIVE(c.green());
            typename _CSTraits::channels_type blue = UINT8_TO_NATIVE(c.blue());
            dst->Y = _CSTraits::computeY( red, green, blue);
            dst->Cb = _CSTraits::computeCb( red, green, blue);
            dst->Cr = _CSTraits::computeCr( red, green, blue);
        }

        virtual void fromQColor(const QColor& c, quint8 opacity, quint8 *dstU8, KoColorProfile * /*profile*/) const
        {
            typename _CSTraits::Pixel *dst = reinterpret_cast<typename _CSTraits::Pixel *>(dstU8);
            typename _CSTraits::channels_type red = UINT8_TO_NATIVE(c.red());
            typename _CSTraits::channels_type green = UINT8_TO_NATIVE(c.green());
            typename _CSTraits::channels_type blue = UINT8_TO_NATIVE(c.blue());
            dst->Y = _CSTraits::computeY( red, green, blue);
            dst->Cb = _CSTraits::computeCb( red, green, blue);
            dst->Cr = _CSTraits::computeCr( red, green, blue);
            dst->alpha = UINT8_TO_NATIVE(opacity);
        }

        virtual void toQColor(const quint8 *srcU8, QColor *c, KoColorProfile * /*profile*/) const
        {
            const typename _CSTraits::Pixel* src = reinterpret_cast<const typename _CSTraits::Pixel *>(srcU8);
            c->setRgb(
                NATIVE_TO_UINT8(_CSTraits::computeRed( src->Y, src->Cb, src->Cr)),
                NATIVE_TO_UINT8(_CSTraits::computeGreen( src->Y, src->Cb, src->Cr)),
                NATIVE_TO_UINT8(_CSTraits::computeBlue( src->Y, src->Cb, src->Cr) ) );
        }

        virtual void toQColor(const quint8 *srcU8, QColor *c, quint8 *opacity, KoColorProfile * /*profile*/) const
        {
            const typename _CSTraits::Pixel* src = reinterpret_cast<const typename _CSTraits::Pixel *>(srcU8);
            c->setRgb(
                NATIVE_TO_UINT8(_CSTraits::computeRed( src->Y, src->Cb, src->Cr)),
                NATIVE_TO_UINT8(_CSTraits::computeGreen( src->Y, src->Cb, src->Cr)),
                NATIVE_TO_UINT8(_CSTraits::computeBlue( src->Y, src->Cb, src->Cr) ) );
            *opacity = NATIVE_TO_UINT8(src->alpha);
        }

        virtual void fromRgbA16(const quint8 * srcU8, quint8 * dstU8, const quint32 nPixels) const
        {
            typename _CSTraits::Pixel* dst = reinterpret_cast<typename _CSTraits::Pixel*>(dstU8);
            const quint16* src = reinterpret_cast<const quint16*>(srcU8);
            typename _CSTraits::channels_type red = UINT16_TO_NATIVE(src[KoRgbU16Traits::red_pos]);
            typename _CSTraits::channels_type green = UINT16_TO_NATIVE(src[KoRgbU16Traits::green_pos]);
            typename _CSTraits::channels_type blue = UINT16_TO_NATIVE(src[KoRgbU16Traits::blue_pos]);
            dst->Y = _CSTraits::computeY( red, green, blue);
            dst->Cb = _CSTraits::computeCb( red, green, blue);
            dst->Cr = _CSTraits::computeCr( red, green, blue);
            dst->alpha = UINT16_TO_NATIVE(src[KoRgbU16Traits::alpha_pos]);
        }
        virtual void toRgbA16(const quint8 * srcU8, quint8 * dstU8, const quint32 nPixels) const
        {
            const typename _CSTraits::Pixel* src = reinterpret_cast< const typename _CSTraits::Pixel*>(srcU8);
            quint16* dst = reinterpret_cast<quint16*>(dstU8);
            dst[ KoRgbU16Traits::red_pos ] = NATIVE_TO_UINT16(_CSTraits::computeRed( src->Y, src->Cb, src->Cr));
            dst[ KoRgbU16Traits::green_pos ] = NATIVE_TO_UINT16(_CSTraits::computeGreen( src->Y, src->Cb, src->Cr));
            dst[ KoRgbU16Traits::blue_pos ] = NATIVE_TO_UINT16(_CSTraits::computeBlue( src->Y, src->Cb, src->Cr));
            dst[KoRgbU16Traits::alpha_pos] = NATIVE_TO_UINT16(src->alpha);
        }
    private:
};

#endif
