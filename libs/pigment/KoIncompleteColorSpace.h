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

#ifndef _KO_INCOMPLETE_COLOR_SPACE_H_
#define _KO_INCOMPLETE_COLOR_SPACE_H_

#include <KoColorSpaceAbstract.h>

/**
 * Inherits this colorspace if you can't provide all the functions defined in KoColorSpace
 * and that you colorspace is unsupported by LCMS.
 */
template<class _CSTraits, class _fallback_>
class KoIncompleteColorSpace : public KoColorSpaceAbstract<_CSTraits> {
    protected:
        KoIncompleteColorSpace(const QString &id, const QString &name, KoColorSpaceRegistry * parent, quint32 type,
                         icColorSpaceSignature colorSpaceSignature) : KoColorSpaceAbstract<_CSTraits>(id, name, parent, type, colorSpaceSignature), m_fallBackColorSpace(_fallback_::createColorSpace())
        {
        }
        virtual ~KoIncompleteColorSpace()
        {
          delete m_fallBackColorSpace;
        }
    public:
        
        virtual bool hasHighDynamicRange() const { return false; }
        virtual KoColorProfile * profile() const { return 0; };
        
        virtual void fromQColor(const QColor& color, quint8 *dst, KoColorProfile * profile=0) const
        {
        }

        virtual void fromQColor(const QColor& color, quint8 opacity, quint8 *dst, KoColorProfile * profile=0) const
        {
        }

        virtual void toQColor(const quint8 *src, QColor *c, KoColorProfile * profile =0) const
        {
        }

        virtual void toQColor(const quint8 *src, QColor *c, quint8 *opacity, KoColorProfile * profile =0) const
        {
        }
        virtual QImage convertToQImage(const quint8 *data, qint32 width, qint32 height,
                KoColorProfile *dstProfile,
                qint32 renderingIntent, float exposure) const

        {
        }
        virtual void toLabA16(const quint8 * src, quint8 * dst, const quint32 nPixels) const
        {
        }

        virtual void fromLabA16(const quint8 * src, quint8 * dst, const quint32 nPixels) const
        {
        }
        virtual void fromRgbA16(const quint8 * src, quint8 * dst, const quint32 nPixels) const
        {
        }
        virtual void toRgbA16(const quint8 * src, quint8 * dst, const quint32 nPixels) const
        {
        }

        virtual bool convertPixelsTo(const quint8 * src,
                quint8 * dst,
                const KoColorSpace * dstColorSpace,
                quint32 numPixels,
                qint32 renderingIntent) const
        {
        }

        virtual KoColorTransformation *createBrightnessContrastAdjustment(quint16 *transferValues) const
        {
          return _fallback_::createTransformation(this, m_fallBackColorSpace, m_fallBackColorSpace->createBrightnessContrastAdjustment( transferValues ));
        }


        virtual KoColorTransformation *createDesaturateAdjustment() const
        {
          return _fallback_::createTransformation(this, m_fallBackColorSpace, m_fallBackColorSpace->createDesaturateAdjustment());
        }

        virtual KoColorTransformation *createPerChannelAdjustment(quint16 **transferValues) const
        {
          return _fallback_::createTransformation(this, m_fallBackColorSpace, m_fallBackColorSpace->createPerChannelAdjustment( transferValues ));
        }

        virtual void invertColor(quint8 * src, qint32 nPixels) const
        {
        }

        virtual quint8 difference(const quint8* src1, const quint8* src2) const
        {
        }

        void darken(const quint8 * src, quint8 * dst, qint32 shade, bool compensate, double compensation, qint32 nPixels) const
        {
        }

    private:
      KoColorSpace* m_fallBackColorSpace;
      quint32 m_cmType;
};

#endif
