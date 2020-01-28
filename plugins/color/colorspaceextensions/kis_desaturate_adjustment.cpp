/*
*  Copyright (c) 2013 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; version 2
 * of the License.
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

#include "kis_desaturate_adjustment.h"

#include <KoConfig.h>

#include <kis_debug.h>
#include <klocalizedstring.h>

#include <KoColorConversions.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpace.h>
#include <KoColorSpaceTraits.h>
#include <KoColorTransformation.h>
#include <KoID.h>

#define SCALE_TO_FLOAT( v ) KoColorSpaceMaths< _channel_type_, float>::scaleToA( v )
#define SCALE_FROM_FLOAT( v  ) KoColorSpaceMaths< float, _channel_type_>::scaleToA( v )

template<typename _channel_type_,typename traits>
class KisDesaturateAdjustment : public KoColorTransformation
{
    typedef traits RGBTrait;
    typedef typename RGBTrait::Pixel RGBPixel;

public:
    KisDesaturateAdjustment()
    {
    }

public:

    void transform(const quint8 *srcU8, quint8 *dstU8, qint32 nPixels) const override
    {
        const RGBPixel* src = reinterpret_cast<const RGBPixel*>(srcU8);
        RGBPixel* dst = reinterpret_cast<RGBPixel*>(dstU8);
        float r, g, b, gray;
        while (nPixels > 0) {

            r = SCALE_TO_FLOAT(src->red);
            g = SCALE_TO_FLOAT(src->green);
            b = SCALE_TO_FLOAT(src->blue);

            // https://www.tannerhelland.com/3643/grayscale-image-algorithm-vb6/
            switch(m_type) {
            case 0: // lightness
            {
                gray = (qMax(qMax(r, g), b) + qMin(qMin(r, g), b)) / 2;
                break;
            }
            case 1: // luminosity BT 709
            {
                gray = r * 0.2126 + g * 0.7152 + b * 0.0722;
                break;

            }

            case 2: // luminosity BT 601
            {
                gray = r * 0.299 + g * 0.587 + b * 0.114;
                break;

            }
            case 3: // average
            {
                gray = (r + g + b) / 3;
                break;
            }
            case 4: // min
            {
                gray = qMin(qMin(r, g), b);
                break;
            }
            case 5: // min
            {
                gray = qMax(qMax(r, g), b);
                break;
            }

            default:
                gray = 0;
            }
            dst->red = SCALE_FROM_FLOAT(gray);
            dst->green = SCALE_FROM_FLOAT(gray);
            dst->blue = SCALE_FROM_FLOAT(gray);
            dst->alpha = src->alpha;

            --nPixels;
            ++src;
            ++dst;
        }
    }

    QList<QString> parameters() const override
    {
      QList<QString> list;
      list << "type";
      return list;
    }

    int parameterId(const QString& name) const override
    {
        if (name == "type") {
            return 0;
        }
        return -1;
    }

    /**
    * name - "type":
    *  0: lightness
    *  1: luminosity
    *  2: average
    */
    void setParameter(int id, const QVariant& parameter) override
    {
        switch(id)
        {
        case 0:
            m_type = parameter.toDouble();
            break;
        default:
            ;
        }
    }

private:

    int m_type;

};


KisDesaturateAdjustmentFactory::KisDesaturateAdjustmentFactory()
    : KoColorTransformationFactory("desaturate_adjustment")
{
}

QList< QPair< KoID, KoID > > KisDesaturateAdjustmentFactory::supportedModels() const
{
    QList< QPair< KoID, KoID > > l;
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Integer8BitsColorDepthID));
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Integer16BitsColorDepthID));
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Float16BitsColorDepthID));
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Float32BitsColorDepthID));
    return l;
}

KoColorTransformation* KisDesaturateAdjustmentFactory::createTransformation(const KoColorSpace* colorSpace, QHash<QString, QVariant> parameters) const
{
    KoColorTransformation * adj;
    if (colorSpace->colorModelId() != RGBAColorModelID) {
        dbgKrita << "Unsupported color space " << colorSpace->id() << " in KisDesaturateAdjustmentFactory::createTransformation";
        return 0;
    }
    if (colorSpace->colorDepthId() == Integer8BitsColorDepthID) {
        adj = new KisDesaturateAdjustment< quint8, KoBgrTraits < quint8 > >();
    } else if (colorSpace->colorDepthId() == Integer16BitsColorDepthID) {
        adj = new KisDesaturateAdjustment< quint16, KoBgrTraits < quint16 > >();
    }
#ifdef HAVE_OPENEXR
    else if (colorSpace->colorDepthId() == Float16BitsColorDepthID) {
        adj = new KisDesaturateAdjustment< half, KoRgbTraits < half > >();
    }
#endif
    else if (colorSpace->colorDepthId() == Float32BitsColorDepthID) {
        adj = new KisDesaturateAdjustment< float, KoRgbTraits < float > >();
    }
    else {
        dbgKrita << "Unsupported color space " << colorSpace->id() << " in KisDesaturateAdjustmentFactory::createTransformation";
        return 0;
    }
    adj->setParameters(parameters);
    return adj;

}
