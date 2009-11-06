/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2007 Emanuele Tamponi <emanuele@valinor.it>
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

#ifndef KOCOLORSPACEABSTRACT_H
#define KOCOLORSPACEABSTRACT_H

#include <QtCore/QBitArray>
#include <klocale.h>

#include <KoColorSpace.h>
#include "KoColorSpaceConstants.h"
#include <KoColorSpaceMaths.h>
#include <KoColorSpaceRegistry.h>
#include <KoIntegerMaths.h>
#include "KoCompositeOp.h"
#include "KoColorTransformation.h"
#include "KoFallBackColorTransformation.h"
#include "KoLabDarkenColorTransformation.h"
#include "KoMixColorsOpImpl.h"

class CompositeCopy : public KoCompositeOp {

    using KoCompositeOp::composite;

    public:

        explicit CompositeCopy(KoColorSpace * cs)
        : KoCompositeOp(cs, COMPOSITE_COPY, i18n("Copy" ), KoCompositeOp::categoryMix() )
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

            qint32 srcInc = (srcRowStride == 0) ? 0 : colorSpace()->pixelSize();

            quint8 *dst = dstRowStart;
            const quint8 *src = srcRowStart;
            const KoColorSpace* cs = colorSpace();
            qint32 bytesPerPixel = cs->pixelSize();

            while (rows > 0) {
                if(srcInc == 0)
                {
                    quint8* dstN = dst;
                    qint32 columns = numColumns;
                    while (columns > 0) {
                      memcpy( dstN, src, bytesPerPixel);
                      dst += bytesPerPixel;
                    }
                } else {
                    memcpy(dst, src, numColumns * bytesPerPixel);
                }

                if (opacity != OPACITY_OPAQUE) {
                    cs->multiplyAlpha(dst, opacity, numColumns);
                }

                dst += dstRowStride;
                src += srcRowStride;
                --rows;
            }
        }
};

template<class _CSTrait>
class KoConvolutionOpImpl : public KoConvolutionOp {
    typedef typename KoColorSpaceMathsTraits<typename _CSTrait::channels_type>::compositetype compositetype;
    typedef typename _CSTrait::channels_type channels_type;
public:

    KoConvolutionOpImpl() { }

    virtual ~KoConvolutionOpImpl() { }

    /**
     * Calculates a weighted average of the pixels, mentioned in @colors
     * using weight values from @kernelValues
     *
     * Note:
     * It behaves in a quite unclear way, when at least one pixel is
     * fully transparent. There are three cases:
     * Case A) None of the pixels is fully transparent.
     *    * Every color channel AND alpha channel of @dst stores a sum
     *      of the corresponding channels from @colors, divided by @factor
     *      and incremented by @offset
     * Case B) At least one pixel of @colors is transparent and @factor
     * stores a weight of the kernel (sum of it's items).
     *    * Every color channel of @dst stores a sum of the corresponding
     *      channels from non-transparent pixels, divided by a weight
     *      of non-transparent pixels and incremented by @offset.
     *    * Alpha channel of @dst stores a sum of the corresponding
     *      channels from non-transparent pixels, divided by a weight
     *      of all the pixels (equals to @factor) and incremented
     *      by @offset.
     * Case C) At least one pixel of @colors is transparent and @factor
     * is set to an arbitrary value.
     *    * Every color channel of @dst stores a sum of the corresponding
     *      channels from non-transparent pixels, divided by a "scaled
     *      down factor" and incremented by @offset. "Scaled
     *      down factor" is calculated in the following way:
     *
     *                                   [weight of non-transparent pixels]
     *      scaledDownFactor = @factor * ----------------------------------
     *                                       [weight of all the pixels]
     *
     *    * Alpha channel of @dst stores a sum of the corresponding
     *      channels from non-transparent pixels, divided by unscaled
     *      @factor and incremented by @offset.
     */

    virtual void convolveColors(const quint8* const* colors, const qint32* kernelValues, quint8 *dst, qint32 factor, qint32 offset, qint32 nPixels, const QBitArray & channelFlags) const
        {

            // Create and initialize to 0 the array of totals
            compositetype totals[_CSTrait::channels_nb];

            qint32 totalWeight = 0;
            qint32 totalWeightTransparent = 0;

            memset(totals, 0, sizeof(typename KoColorSpaceMathsTraits<typename _CSTrait::channels_type>::compositetype) * _CSTrait::channels_nb);

            for (;nPixels--; colors++, kernelValues++)
            {
                qint32 weight = *kernelValues;
                const channels_type* color = _CSTrait::nativeArray(*colors);
                if( weight != 0 )
                {
                    if( _CSTrait::alpha( *colors ) == 0 )
                    {
                        totalWeightTransparent += weight;
                    } else {
                        for(uint i = 0; i < _CSTrait::channels_nb; i++)
                        {
                            totals[i] += color[i] * weight;
                        }
                    }
                    totalWeight += weight;
                }
            }

            typename _CSTrait::channels_type* dstColor = _CSTrait::nativeArray(dst);

            bool allChannels = channelFlags.isEmpty();
            Q_ASSERT( allChannels || channelFlags.size() == (int)_CSTrait::channels_nb );
            if(totalWeightTransparent == 0)
            {
                // Case A)
                for (uint i = 0; i < _CSTrait::channels_nb; i++)
                {
                    if ( allChannels || channelFlags.testBit( i ) )
                    {
                        compositetype v = totals[i] / factor + offset;
                        dstColor[ i ] = CLAMP(v, KoColorSpaceMathsTraits<channels_type>::min,
                                                 KoColorSpaceMathsTraits<channels_type>::max );
                    }
                }
            }
            else if (totalWeightTransparent != totalWeight ) {
                if(totalWeight == factor)
                {
                    // Case B)
                    qint64 a = ( totalWeight - totalWeightTransparent );
                    for(uint i = 0; i < _CSTrait::channels_nb; i++)
                    {
                        if( allChannels || channelFlags.testBit( i ) )
                        {
                            if( i == (uint)_CSTrait::alpha_pos )
                            {
                                compositetype v = totals[i] / totalWeight + offset;
                                dstColor[ i ] = CLAMP(v, KoColorSpaceMathsTraits<channels_type>::min,
                                                         KoColorSpaceMathsTraits<channels_type>::max );
                            } else {
                                compositetype v = totals[i] / a + offset;
                                dstColor[ i ] = CLAMP(v, KoColorSpaceMathsTraits<channels_type>::min,
                                                         KoColorSpaceMathsTraits<channels_type>::max );
                            }
                        }
                    }
               } else {
                    // Case C)
                    qreal a = qreal(totalWeight) / ( factor * ( totalWeight - totalWeightTransparent ) ); // use qreal as it easily saturate
                    for(uint i = 0; i < _CSTrait::channels_nb; i++)
                    {
                        if( allChannels || channelFlags.testBit( i ) )
                        {
                            if( i == (uint)_CSTrait::alpha_pos )
                            {
                                compositetype v = totals[i] / factor + offset;
                                dstColor[ i ] = CLAMP(v, KoColorSpaceMathsTraits<channels_type>::min,
                                                         KoColorSpaceMathsTraits<channels_type>::max );
                            } else {
                                compositetype v = (compositetype)( totals[i] * a + offset );
                                dstColor[ i ] = CLAMP(v, KoColorSpaceMathsTraits<channels_type>::min,
                                                         KoColorSpaceMathsTraits<channels_type>::max );
                            }
                        }
                    }
                }
            }

        }
};

class KoInvertColorTransformation : public KoColorTransformation {

    public:

        KoInvertColorTransformation(const KoColorSpace* cs) : m_colorSpace(cs), m_psize(cs->pixelSize())
        {
        }

        virtual void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const
        {
            quint16 m_rgba[4];
            while(nPixels--)
            {
                m_colorSpace->toRgbA16(src, reinterpret_cast<quint8 *>(m_rgba), 1);
                m_rgba[0] = KoColorSpaceMathsTraits<quint16>::max - m_rgba[0];
                m_rgba[1] = KoColorSpaceMathsTraits<quint16>::max - m_rgba[1];
                m_rgba[2] = KoColorSpaceMathsTraits<quint16>::max - m_rgba[2];
                m_colorSpace->fromRgbA16(reinterpret_cast<quint8 *>(m_rgba), dst, 1);
                src += m_psize;
                dst += m_psize;
            }

        }

    private:

        const KoColorSpace* m_colorSpace;
        quint32 m_psize;
};

/**
 * This in an implementation of KoColorSpace which can be used as a base for colorspaces with as many
 * different channels of the same type.
 * the template parameters must be a class which inherits KoColorSpaceTrait (or a class with the same signature).
 * Where SOMETYPE is the type of the channel for instance (quint8, quint32...), SOMENBOFCHANNELS is the number of channels
 * including the alpha channel, SOMEALPHAPOS is the position of the alpha channel in the pixel (can be equal to -1 if no
 * alpha channel).
 */

template<class _CSTrait>
class KoColorSpaceAbstract : public KoColorSpace {
    public:
        KoColorSpaceAbstract(const QString &id, const QString &name) :
            KoColorSpace(id, name, new KoMixColorsOpImpl< _CSTrait>(), new KoConvolutionOpImpl< _CSTrait>())
        {
            this->addCompositeOp( new CompositeCopy( this ) );
        }

        virtual quint32 colorChannelCount() const
        {
          if(_CSTrait::alpha_pos == -1 )
            return _CSTrait::channels_nb;
          else
            return _CSTrait::channels_nb - 1;
        }
        virtual quint32 channelCount() const { return _CSTrait::channels_nb; }
        virtual quint32 pixelSize() const { return _CSTrait::pixelSize; }

        virtual QString channelValueText(const quint8 *pixel, quint32 channelIndex) const
        {
            return _CSTrait::channelValueText(pixel, channelIndex);
        }

        virtual QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const
        {
            return _CSTrait::normalisedChannelValueText(pixel, channelIndex);
        }

        virtual void normalisedChannelsValue(const quint8 *pixel, QVector<float> &channels) const
        {
            return _CSTrait::normalisedChannelsValue(pixel, channels);
        }

        virtual void fromNormalisedChannelsValue(quint8 *pixel, const QVector<float> &values) const
        {
            return _CSTrait::fromNormalisedChannelsValue(pixel, values);
        }

        virtual quint8 scaleToU8(const quint8 * srcPixel, qint32 channelIndex) const {
            typename _CSTrait::channels_type c = _CSTrait::nativeArray(srcPixel)[channelIndex];
            return KoColorSpaceMaths<typename _CSTrait::channels_type, quint8>::scaleToA(c);
        }

        virtual quint16 scaleToU16(const quint8 * srcPixel, qint32 channelIndex) const {
            typename _CSTrait::channels_type c = _CSTrait::nativeArray(srcPixel)[channelIndex];
            return KoColorSpaceMaths<typename _CSTrait::channels_type,quint16>::scaleToA(c);
        }
        virtual void singleChannelPixel(quint8 *dstPixel, const quint8 *srcPixel, quint32 channelIndex) const
        {
            _CSTrait::singleChannelPixel(dstPixel, srcPixel, channelIndex);
        }

        virtual quint8 alpha(const quint8 * U8_pixel) const
        {
            return _CSTrait::alpha(U8_pixel);
        }

        virtual void setAlpha(quint8 * pixels, quint8 alpha, qint32 nPixels) const
        {
            _CSTrait::setAlpha(pixels, alpha, nPixels);
        }

        virtual void multiplyAlpha(quint8 * pixels, quint8 alpha, qint32 nPixels) const
        {
            _CSTrait::multiplyAlpha( pixels, alpha, nPixels);
        }

        virtual void applyAlphaU8Mask(quint8 * pixels, const quint8 * alpha, qint32 nPixels) const
        {
            _CSTrait::applyAlphaU8Mask(pixels, alpha, nPixels);
        }

        virtual void applyInverseAlphaU8Mask(quint8 * pixels, const quint8 * alpha, qint32 nPixels) const
        {
            _CSTrait::applyInverseAlphaU8Mask(pixels, alpha, nPixels);
        }

        virtual quint8 intensity8(const quint8 * src) const
        {
            QColor c;
            const_cast<KoColorSpaceAbstract<_CSTrait> *>(this)->toQColor(src, &c );
            return static_cast<quint8>((c.red() * 0.30 + c.green() * 0.59 + c.blue() * 0.11) + 0.5);
        }

        virtual KoColorTransformation* createInvertTransformation() const
        {
            return new KoInvertColorTransformation(this);
        }

         virtual KoColorTransformation *createDarkenAdjustment(qint32 shade, bool compensate, qreal compensation) const
        {
            return new KoFallBackColorTransformation(this, KoColorSpaceRegistry::instance()->lab16(""), new KoLabDarkenColorTransformation<quint16>( shade, compensate, compensation) );
        }

        virtual KoID mathToolboxId() const
        {
            return KoID("Basic");
        }
};


#endif
