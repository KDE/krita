/*
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef _KIS_YCBCR_BASE_COLORSPACE_H_
#define _KIS_YCBCR_BASE_COLORSPACE_H_

#include "klocale.h"
#include <KoIncompleteColorSpace.h>
#include <KoColorSpaceTraits.h>
#include <KoColorSpaceRegistry.h>

#define UINT8_TO_NATIVE(v) (KoColorSpaceMaths<quint8, typename _CSTraits::channels_type >::scaleToA(v))
#define NATIVE_TO_UINT8(v) (KoColorSpaceMaths<typename _CSTraits::channels_type, quint8>::scaleToA(v))
#define UINT16_TO_NATIVE(v) (KoColorSpaceMaths<quint16, typename _CSTraits::channels_type >::scaleToA(v))
#define NATIVE_TO_UINT16(v) (KoColorSpaceMaths<typename _CSTraits::channels_type, quint16>::scaleToA(v))


template <class _CSTraits>
class KisYCbCrBaseColorSpace : public KoIncompleteColorSpace<_CSTraits>
{
    public:
        KisYCbCrBaseColorSpace(const QString &id, const QString &name, KoColorSpaceRegistry * parent)
          : KoIncompleteColorSpace<_CSTraits>(id, name, parent, parent->rgb16(""))
        {

        }

        virtual bool profileIsCompatible(KoColorProfile* /*profile*/) const
        {
            return false;
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
    private:
};

#endif
