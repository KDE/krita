/*
 *  Copyright (c) 2013 Sahil Nagpal <nagpal.sahil01@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; version 2
 * of the License, or (at your option) any later version.
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

#include "kis_dodgemidtones_adjustment.h"
#include <KoConfig.h>

#include <kis_debug.h>
#include <klocale.h>

 #include <KoColorConversions.h>
 #include <KoColorModelStandardIds.h>
 #include <KoColorSpace.h>
 #include <KoColorSpaceTraits.h>
 #include <KoColorTransformation.h>
 #include <KoID.h>
 
template<typename _channel_type_>
class KisDodgeMidtonesAdjustment : public KoColorTransformation
 {
    typedef KoBgrTraits<_channel_type_> RGBTrait;
    typedef typename RGBTrait::Pixel RGBPixel;

public:
 	KisDodgeMidtonesAdjustment(){};

public:
    
    void transform(const quint8 *srcU8, quint8 *dstU8, qint32 nPixels) const
    {
    	const RGBPixel* src = reinterpret_cast<const RGBPixel*>(srcU8);
        RGBPixel* dst = reinterpret_cast<RGBPixel*>(dstU8);
        float  factor, value_red, value_green, value_blue;
        while(nPixels > 0) {

            factor = 1.0/(1.0 + exposure);
            value_red = pow(KoColorSpaceMaths<_channel_type_, float>::scaleToA(src->red), factor);
            value_green = pow(KoColorSpaceMaths<_channel_type_, float>::scaleToA(src->green), factor);
            value_blue = pow(KoColorSpaceMaths<_channel_type_, float>::scaleToA(src->blue), factor);
            
            dst->red = KoColorSpaceMaths< float, _channel_type_ >::scaleToA(value_red);
            dst->green = KoColorSpaceMaths< float, _channel_type_ >::scaleToA(value_green);
            dst->blue = KoColorSpaceMaths< float, _channel_type_ >::scaleToA(value_blue);
            dst->alpha = src->alpha;
            
            --nPixels;
            ++src;
            ++dst;
        }
    }

    virtual QList<QString> parameters() const
    {
    	QList<QString> list;
    	list << "exposure";
    	return list;
    }

    virtual int parameterId(const QString& name) const
    {
        if (name == "exposure")
        return 0;
        return -1;
    }

    virtual void setParameter(int id, const QVariant& parameter)
    {
        switch(id)
        {
        case 0:
            exposure = parameter.toDouble();
            break;
        default:
            ;
        }
    }
private:

	float exposure;
};

 KisDodgeMidtonesAdjustmentFactory::KisDodgeMidtonesAdjustmentFactory()
    : KoColorTransformationFactory("DodgeMidtones", i18n("DodgeMidtones Adjustment"))
{
}

QList< QPair< KoID, KoID > > KisDodgeMidtonesAdjustmentFactory::supportedModels() const
{
    QList< QPair< KoID, KoID > > l;
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Integer8BitsColorDepthID));
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Integer16BitsColorDepthID));
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Float32BitsColorDepthID)); 
    return l;
}

KoColorTransformation* KisDodgeMidtonesAdjustmentFactory::createTransformation(const KoColorSpace* colorSpace, QHash<QString, QVariant> parameters) const
{
    KoColorTransformation * adj;
    if (colorSpace->colorModelId() != RGBAColorModelID) {
        kError() << "Unsupported color space " << colorSpace->id() << " in KisDodgeMidtonesAdjustmentFactory::createTransformation";
        return 0;
    }
    if (colorSpace->colorDepthId() == Float32BitsColorDepthID) {
        adj = new KisDodgeMidtonesAdjustment< float >();
    } else if(colorSpace->colorDepthId() == Integer16BitsColorDepthID) {
        adj = new KisDodgeMidtonesAdjustment< quint16 >();
    } else if(colorSpace->colorDepthId() == Integer8BitsColorDepthID) {
        adj = new KisDodgeMidtonesAdjustment< quint8 >();
    } else {
        kError() << "Unsupported color space " << colorSpace->id() << " in KisDodgeMidtonesAdjustmentFactory::createTransformation";
        return 0;
    }
    adj->setParameters(parameters);
    return adj;
}
