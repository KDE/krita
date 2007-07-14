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
#ifndef KIS_GENERIC_COLORSPACE_H_
#define KIS_GENERIC_COLORSPACE_H_

#include <KoColorSpaceAbstract.h>
#include <KoColorSpaceTraits.h>

#include <qimage.h>


/**
 * This is not a real colorspace, but a fake colorspace you might want to use
 * to be able to use kispaintdevice for big buffer of memory.
 */
template<typename _type, int _nbchannels>
class KisGenericColorspace : public KoColorSpace {

    class KisGenericColorspaceConvolutionOpImpl : public KoConvolutionOp{
        public:
            KisGenericColorspaceConvolutionOpImpl() { }
            virtual ~KisGenericColorspaceConvolutionOpImpl() { }
            virtual void convolveColors(quint8** colors, qint32* kernelValues, quint8 *dst, qint32 factor, qint32 offset, qint32 nColors, const QBitArray & channelFlags) const
            {
                _type totals[ _nbchannels ];
                for(uint i = 0; i < _nbchannels; i++)
                {
                    totals[ i ] = 0;
                }
    
                _type** colorsT = reinterpret_cast<_type**>( colors );
                _type* dstT = reinterpret_cast<_type*>( dst );
                
                while (nColors--)
                {
                    Q_INT32 weight = *kernelValues;
    
                    if (weight != 0) {
                        for(uint i = 0; i < _nbchannels; i++)
                        {
                            totals[ i ] += (*colorsT)[ i ] * weight ;
                        }
                    }
                    colorsT++;
                    kernelValues++;
                }
                for(uint i = 0; i < _nbchannels; i++)
                {
                    dstT[ i ] = totals[ i ] / factor + offset ;
                }
            }
    };


    public:

        KisGenericColorspace() :
        KoColorSpace("genericcolorspace", "genericcolorspace",0, 0, new KisGenericColorspaceConvolutionOpImpl)
        { }
        virtual ~KisGenericColorspace() { }

    public:
        virtual bool profileIsCompatible(KoColorProfile* profile) const {return false;}

        //========== Channels =====================================================//

    /// Return a vector describing all the channels this color model has.
        virtual QList<KoChannelInfo *> channels() const { return QList<KoChannelInfo *>(); }

        virtual quint32 channelCount() const { return _nbchannels; }

        virtual quint32 colorChannelCount() const { return _nbchannels; };

        virtual quint32 substanceChannelCount() const { return 0; };
        
        virtual quint32 pixelSize() const { return _nbchannels * sizeof(_type); }

        virtual KoColorProfile * profile() const { return 0; }

        virtual bool willDegrade(ColorSpaceIndependence ) const {return true;}

        virtual QString channelValueText(const quint8 */*pixel*/, quint32 /*channelIndex*/) const { return "invalid"; };

        virtual QString normalisedChannelValueText(const quint8 */*pixel*/, quint32 /*channelIndex*/) const { return "invalid"; };

        virtual quint8 scaleToU8(const quint8 * /*srcPixel*/, qint32 /*channelPos*/) const { return 0; }

        virtual quint16 scaleToU16(const quint8 * /*srcPixel*/, qint32 /*channelPos*/) const { return 0; }

        virtual void getSingleChannelPixel(quint8 */*dstPixel*/, const quint8 */*srcPixel*/, quint32 /*channelIndex*/) { }

        virtual quint32 colorSpaceType() { return 0; }

        virtual icColorSpaceSignature colorSpaceSignature() { return icMaxEnumData; }

        virtual bool willDegrade(ColorSpaceIndependence /*independence*/) { return true; }
    
        virtual bool hasHighDynamicRange() const { return false; }

        virtual KoColorProfile * getProfile() const { return 0; }

        virtual void fromQColor(const QColor& /*c*/, quint8 */*dst*/, KoColorProfile */* profile = 0*/) const { }

        virtual void fromQColor(const QColor& /*c*/, quint8 /*opacity*/, quint8 */*dst*/, KoColorProfile * /*profile = 0*/) const { }

        virtual void toQColor(const quint8 */*src*/, QColor */*c*/, KoColorProfile * /*profile = 0*/) const { }
        
        virtual void toQColor(const quint8 */*src*/, QColor */*c*/, quint8 */*opacity*/, KoColorProfile * /*profile = 0*/) const { }

        virtual void singleChannelPixel(quint8 *, const quint8 *, quint32 ) const {}

        virtual QImage convertToQImage(const quint8 */*data*/, qint32 /*width*/, qint32 /*height*/,
                                       KoColorProfile *  /*dstProfile*/, KoColorConvertionTransformation::Intent /*renderingIntent = INTENT_PERCEPTUAL*/,
                                       float /*exposure = 0.0f*/) const { return QImage(); }

        virtual void toLabA16(const quint8 * /*src*/, quint8 * /*dst*/, const quint32 /*nPixels*/) const { }

        virtual void fromLabA16(const quint8 * /*src*/, quint8 * /*dst*/, const quint32 /*nPixels*/) const { }
        virtual void toRgbA16(const quint8 * , quint8 * , const quint32 ) const {}
        virtual void fromRgbA16(const quint8 * , quint8 * , const quint32 ) const {}

        virtual bool convertPixelsTo(const quint8 * /*src*/,
                                     quint8 * /*dst*/, KoColorSpace * /*dstColorSpace*/,
                                     quint32 /*numPixels*/,
                                     KoColorConvertionTransformation::Intent /*renderingIntent = INTENT_PERCEPTUAL*/) const { return false; }

        virtual quint8 alpha(const quint8 * /*pixel*/) const { return 0; }

        virtual void setAlpha(quint8 * /*pixels*/, quint8 /*alpha*/, qint32 /*nPixels*/) const { }
        
        virtual void multiplyAlpha(quint8 * /*pixels*/, quint8 /*alpha*/, qint32 /*nPixels*/) const { }

        virtual void applyAlphaU8Mask(quint8 * /*pixels*/, const quint8 * /*alpha*/, qint32 /*nPixels*/) const { }

        virtual void applyInverseAlphaU8Mask(quint8 * /*pixels*/, const quint8 * /*alpha*/, qint32 /*nPixels*/) const { }

        virtual quint8 difference(const quint8* /*src1*/, const quint8* /*src2*/) const { return 255; }

        virtual void mixColors(const quint8 **/*colors*/, const quint8 */*weights*/, quint32 /*nColors*/, quint8 */*dst*/) const { }

        virtual void convolveColors(quint8** colors, qint32* kernelValues, quint8 *dst, qint32 factor, qint32 offset, qint32 nColors, const QBitArray & channelFlags) const
        {
            _type totals[ _nbchannels ];
            for(uint i = 0; i < _nbchannels; i++)
            {
                totals[ i ] = 0;
            }

            _type** colorsT = reinterpret_cast<_type**>( colors );
            _type* dstT = reinterpret_cast<_type*>( dst );
            
            while (nColors--)
            {
                qint32 weight = *kernelValues;

                if (weight != 0) {
                    for(uint i = 0; i < _nbchannels; i++)
                    {
                        totals[ i ] += (*colorsT)[ i ] * weight ;
                    }
                }
                colorsT++;
                kernelValues++;
            }
            for(uint i = 0; i < _nbchannels; i++)
            {
                dstT[ i ] = totals[ i ] / factor + offset ;
            }
        }
        virtual KoColorTransformation *createBrightnessContrastAdjustment(quint16 *) const
        {
            return 0;
        }
        
        virtual KoColorTransformation *createDesaturateAdjustment() const
        {
            return 0;
        }

        virtual KoColorTransformation *createInvertTransformation() const
        {
            return 0;
        }

        virtual KoColorTransformation *createDarkenAdjustement(qint32 , bool , double ) const
        {
            return 0;
        }

        virtual KoColorTransformation *createPerChannelAdjustment(quint16 **) const
        {
            return 0;
        }

        virtual quint8 intensity8(const quint8 * /*src*/) const { return 0; }
    
        virtual KoID mathToolboxId() const { return KoID("",""); }
    
        virtual void bitBlt(quint8 */*dst*/,
                            qint32 /*dststride*/,
                            KoColorSpace * /*srcSpace*/,
                            const quint8 */*src*/,
                            qint32 /*srcRowStride*/,
                            const quint8 */*srcAlphaMask*/,
                            qint32 /*maskRowStride*/,
                            quint8 /*opacity*/,
                            qint32 /*rows*/,
                            qint32 /*cols*/,
                            const KoCompositeOp& /*op*/) { }

        virtual QList<KisFilter*> createBackgroundFilters()
        { return QList<KisFilter*>(); };
};

#endif // KIS_COLORSPACE_H_
