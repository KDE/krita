/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOCOLORSPACEABSTRACT_H
#define KOCOLORSPACEABSTRACT_H

#include <klocale.h>

#include <KoColorSpace.h>
#include <KoColorSpaceMaths.h>

namespace {

    class CompositeCopy : public KoCompositeOp {

        public:

            CompositeCopy(KoColorSpace * cs)
            : KoCompositeOp(cs, COMPOSITE_OVER, i18n("Copy" ) )
            {
            }

        public:

            void composite(quint8 *dstRowStart,
                           qint32 dstRowStride,
                           const quint8 *srcRowStart,
                           qint32 srcRowStride,
                           const quint8 *maskRowStart,
                           qint32 maskRowStride,
                           qint32 rows,
                           qint32 numColumns,
                           quint8 opacity,
                           const QBitArray & channelFlags) const
            {

                Q_UNUSED( maskRowStart );
                Q_UNUSED( maskRowStride );
                Q_UNUSED( channelFlags );
                quint8 *dst = dstRowStart;
                const quint8 *src = srcRowStart;
                qint32 bytesPerPixel = m_colorSpace->pixelSize();

                while (rows > 0) {
                    memcpy(dst, src, numColumns * bytesPerPixel);

                    if (opacity != OPACITY_OPAQUE) {
                        m_colorSpace->multiplyAlpha(dst, opacity, numColumns);
                    }

                    dst += dstRowStride;
                    src += srcRowStride;
                    --rows;
                }
            }
    };
}

template<typename _Tchannels, quint32 _nchannels>
class KoColorSpaceAbstract : public KoColorSpace {
    public:
        KoColorSpaceAbstract(const QString &id, const QString &name, KoColorSpaceRegistry * parent, qint32 alphaPos) : KoColorSpace(id, name, parent), m_alphaPos(alphaPos) {
            this->m_compositeOps.insert( COMPOSITE_COPY, new CompositeCopy( this ) );
        };
        
        virtual quint32 nColorChannels() const { return _nchannels - 1; }
        virtual quint32 nChannels() const { return _nchannels; };
        virtual quint32 pixelSize() const { return _nchannels * sizeof(_nchannels); }

        Q3ValueVector<KoChannelInfo *> channels()
        {
            return m_channels;
        }
        virtual quint32 pixelSize()
        {
            return _nchannels * sizeof(_Tchannels);
        }

        virtual QString channelValueText(const quint8 *pixel, quint32 channelIndex) const
        {
            if(channelIndex > _nchannels) return QString("Error");
            _Tchannels c = nativeArray(pixel)[channelIndex];
            return QString().setNum(c);
        }

        virtual QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const
        {
            if(channelIndex > _nchannels) return QString("Error");
            _Tchannels c = nativeArray(pixel)[channelIndex];
            return QString().setNum( 100. * ((double)c ) / KoColorSpaceMathsTraits<_Tchannels>::max() );
        }
        
        virtual quint8 scaleToU8(const quint8 * srcPixel, qint32 channelIndex) {
            _Tchannels c = nativeArray(srcPixel)[channelIndex];
            return KoColorSpaceMaths<_Tchannels, quint8>::scaleToA(c);
        }

        virtual quint16 scaleToU16(const quint8 * srcPixel, qint32 channelIndex) {
            _Tchannels c = nativeArray(srcPixel)[channelIndex];
            return KoColorSpaceMaths<_Tchannels,quint16>::scaleToA(c);
        }
        virtual void getSingleChannelPixel(quint8 *dstPixel, const quint8 *srcPixel, quint32 channelIndex) 
        {
            const _Tchannels* src = nativeArray(srcPixel);
            _Tchannels* dst = nativeArray(dstPixel);
            for(uint i = 0; i < _nchannels;i++)
            {
                if( i != channelIndex )
                {
                    dst[i] = 0;
                } else {
                    dst[i] = src[i];
                }
            }
        }
        virtual quint8 getAlpha(const quint8 * U8_pixel) const
        {
            if (m_alphaPos < 0) return OPACITY_OPAQUE;
            _Tchannels c = nativeArray(U8_pixel)[m_alphaPos];
            return  KoColorSpaceMaths<_Tchannels,quint8>::scaleToA(c);
        }
        virtual void setAlpha(quint8 * pixels, quint8 alpha, qint32 nPixels) const
        {
            if (m_alphaPos < 0) return;
            qint32 psize = pixelSize();
            _Tchannels valpha =  KoColorSpaceMaths<quint8,_Tchannels>::scaleToA(alpha);
            for (; nPixels > 0; --nPixels, pixels += psize) {
                nativeArray(pixels)[m_alphaPos] = valpha;
            }
        }
        virtual void multiplyAlpha(quint8 * pixels, quint8 alpha, qint32 nPixels)
        {
            if (m_alphaPos < 0) return;

            qint32 psize = pixelSize();
            _Tchannels valpha =  KoColorSpaceMaths<quint8,_Tchannels>::scaleToA(alpha);

            for (; nPixels > 0; --nPixels, pixels += psize) {
                _Tchannels* alphapixel = nativeArray(pixels) + m_alphaPos;
                *alphapixel = KoColorSpaceMaths<_Tchannels>::multiply( *alphapixel, valpha );
            }
        }

        virtual void applyAlphaU8Mask(quint8 * pixels, quint8 * alpha, qint32 nPixels) 
        {
            if (m_alphaPos < 0) return;
            qint32 psize = pixelSize();

            for (; nPixels > 0; --nPixels, pixels += psize, ++alpha) {
                _Tchannels valpha =  KoColorSpaceMaths<quint8,_Tchannels>::scaleToA(*alpha);
                _Tchannels* alphapixel = nativeArray(pixels) + m_alphaPos;
                *alphapixel = KoColorSpaceMaths<_Tchannels>::multiply( *alphapixel, valpha );
            }
        }
        
        virtual void applyInverseAlphaU8Mask(quint8 * pixels, quint8 * alpha, qint32 nPixels)
        {
            if (m_alphaPos < 0) return;
            qint32 psize = pixelSize();

            for (; nPixels > 0; --nPixels, pixels += psize, ++alpha) {
                _Tchannels valpha =  KoColorSpaceMaths<quint8,_Tchannels>::scaleToA(OPACITY_OPAQUE - *alpha);
                _Tchannels* alphapixel = nativeArray(pixels) + m_alphaPos;
                *alphapixel = KoColorSpaceMaths<_Tchannels>::multiply( *alphapixel, valpha );
            }
        }

        virtual void mixColors(const quint8 **colors, const quint8 *weights, quint32 nColors, quint8 *dst) const
        {
            // Create and initialize to 0 the array of totals
            typename KoColorSpaceMathsTraits<_Tchannels>::compositetype totals[_nchannels];
            typename KoColorSpaceMathsTraits<_Tchannels>::compositetype totalAlpha = 0;
            memset(totals, 0, sizeof(typename KoColorSpaceMathsTraits<_Tchannels>::compositetype) * _nchannels);
            // Compute the total for each channel by summing each colors multiplied by the weight
            while(nColors--)
            {
                const _Tchannels* color = nativeArray(*colors);
                quint8 alphaTimesWeight =  KoColorSpaceMaths<quint8>::multiply(getAlpha(*colors), *weights);
                for(uint i = 0; i < _nchannels; i++)
                {
                    totals[i] += color[i] * alphaTimesWeight;
                }
                totalAlpha += alphaTimesWeight;
                colors++;
                weights++;
            }
            // set totalAlpha to the minimum between its value and the maximum value of the channels
            if (totalAlpha > KoColorSpaceMathsTraits<_Tchannels>::max()) {
                totalAlpha = KoColorSpaceMathsTraits<_Tchannels>::max();
            }
            _Tchannels* dstColor = nativeArray(dst);
            if (totalAlpha > 0) {
                for(uint i = 0; i < _nchannels; i++)
                {
                    typename KoColorSpaceMathsTraits<_Tchannels>::compositetype v = totals[i] / totalAlpha;
                    if(v > KoColorSpaceMathsTraits<_Tchannels>::max()) {
                        v = KoColorSpaceMathsTraits<_Tchannels>::max();
                    }
                    dstColor[ i ] = v;
                }
                dstColor[ m_alphaPos ] = totalAlpha;
            } else {
                memset(dst, 0, sizeof(_Tchannels) * _nchannels);
            }
        }
        
        virtual void convolveColors(quint8** colors, qint32* kernelValues, KoChannelInfo::enumChannelFlags channelFlags, quint8 *dst, qint32 factor, qint32 offset, qint32 nPixels) const
        {
            // Create and initialize to 0 the array of totals
            typename KoColorSpaceMathsTraits<_Tchannels>::compositetype totals[_nchannels];
            typename KoColorSpaceMathsTraits<_Tchannels>::compositetype totalAlpha = 0;
            memset(totals, 0, sizeof(typename KoColorSpaceMathsTraits<_Tchannels>::compositetype) * _nchannels);
            for (;nPixels--; colors++, kernelValues++)
            {
                const _Tchannels* color = nativeArray(*colors);
                quint8 alphaTimesWeight =  KoColorSpaceMaths<quint8>::multiply(getAlpha(*colors), *kernelValues);
                for(uint i = 0; i < _nchannels; i++)
                {
                    totals[i] += color[i] * alphaTimesWeight;
                }
                totalAlpha += alphaTimesWeight;
            }
            
            if (channelFlags & KoChannelInfo::FLAG_COLOR) {
                _Tchannels* dstColor = nativeArray(dst);
                for(uint i = 0; i < _nchannels; i++)
                {
                    typename KoColorSpaceMathsTraits<_Tchannels>::compositetype v = totals[i] / factor + offset;
                    dstColor[ i ] = CLAMP(v, KoColorSpaceMathsTraits<_Tchannels>::min(), KoColorSpaceMathsTraits<_Tchannels>::max());
                }
 
            }
            if (channelFlags & KoChannelInfo::FLAG_ALPHA) {
                setAlpha(dst, CLAMP((totalAlpha/ factor) + offset, 0, SCHAR_MAX ),1);
            }

        }

        virtual quint8 intensity8(const quint8 * src) const
        {
            QColor c;
            quint8 opacity;
            const_cast<KoColorSpaceAbstract<_Tchannels, _nchannels> *>(this)->toQColor(src, &c, &opacity);
            return static_cast<quint8>((c.red() * 0.30 + c.green() * 0.59 + c.blue() * 0.11) + 0.5);
        }

        virtual KoID mathToolboxID() const
        {
            return KoID("Basic");
        }

    private:
        inline const _Tchannels* nativeArray(const quint8 * a) const { return reinterpret_cast<const _Tchannels*>(a); }
        inline _Tchannels* nativeArray(quint8 * a) const { return reinterpret_cast<_Tchannels*>(a); }
   private:
        qint32 m_alphaPos; // The position in _bytes_ of the alpha channel
};


#endif
