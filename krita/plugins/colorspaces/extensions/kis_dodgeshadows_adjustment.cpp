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

#include "kis_dodgeshadows_adjustment.h"
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
class KisDodgeShadowsAdjustment : public KoColorTransformation
{
    typedef KoBgrTraits<_channel_type_> RGBTrait;
    typedef typename RGBTrait::Pixel RGBPixel;

public:
 	KisDodgeShadowsAdjustment(){};

 	void transform(const quint8 *srcU8, quint8 *dstU8, qint32 nPixels) const
 	{
        const RGBPixel* src = reinterpret_cast<const RGBPixel*>(srcU8);
        RGBPixel* dst = reinterpret_cast<RGBPixel*>(dstU8);
        float factor, value_red, value_green, value_blue, new_value_red, new_value_green, new_value_blue;
        while (nPixels > 0) {

            value_red = KoColorSpaceMaths<_channel_type_, float>::scaleToA(src->red);
            value_green = KoColorSpaceMaths<_channel_type_, float>::scaleToA(src->green);
            value_blue = KoColorSpaceMaths<_channel_type_, float>::scaleToA(src->blue);

            factor = exposure * 0.333333;
            new_value_red = factor + value_red  - factor * value_red;
            new_value_green = factor + value_green  - factor * value_green;
            new_value_blue = factor + value_blue  - factor * value_blue;
            
            dst->red = KoColorSpaceMaths< float, _channel_type_ >::scaleToA(new_value_red);
            dst->green = KoColorSpaceMaths< float, _channel_type_ >::scaleToA(new_value_green);
            dst->blue = KoColorSpaceMaths< float, _channel_type_ >::scaleToA(new_value_blue);
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

 KisDodgeShadowsAdjustmentFactory::KisDodgeShadowsAdjustmentFactory()
    : KoColorTransformationFactory("DodgeShadows", i18n("DodgeShadows Adjustment"))
{
}

QList< QPair< KoID, KoID > > KisDodgeShadowsAdjustmentFactory::supportedModels() const
{
    QList< QPair< KoID, KoID > > l;
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Integer8BitsColorDepthID));
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Integer16BitsColorDepthID));
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Float32BitsColorDepthID));
    return l;
}

KoColorTransformation* KisDodgeShadowsAdjustmentFactory::createTransformation(const KoColorSpace* colorSpace, QHash<QString, QVariant> parameters) const
{
    KoColorTransformation * adj;
    if (colorSpace->colorModelId() != RGBAColorModelID) {
        kError() << "Unsupported color space " << colorSpace->id() << " in KisDodgeShadowsAdjustmentFactory::createTransformation";
        return 0;
    }
    if (colorSpace->colorDepthId() == Float32BitsColorDepthID) {
        adj = new KisDodgeShadowsAdjustment < float >();
    } else if (colorSpace->colorDepthId() == Integer16BitsColorDepthID) {
        adj = new KisDodgeShadowsAdjustment< quint16 >();
    } else if (colorSpace->colorDepthId() == Integer8BitsColorDepthID) {
        adj = new KisDodgeShadowsAdjustment< quint8 >();
    } else {
        kError() << "Unsupported color space " << colorSpace->id() << " in KisDodgeShadowsAdjustmentFactory::createTransformation";
        return 0;
    }
    adj->setParameters(parameters);
    return adj;

}
