/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "kis_hsv_adjustment.h"
#include <config-openexr.h>
#ifdef HAVE_OPENEXR
#include <half.h>
#endif

#include <kis_debug.h>
#include <klocale.h>

#include <KoColorConversions.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpace.h>
#include <KoColorSpaceTraits.h>
#include <KoColorTransformation.h>
#include <KoID.h>

#define SCALE_TO_FLOAT( v ) KoColorSpaceMaths< _channel_type_, float>::scaleToA( v )
#define SCALE_FROM_FLOAT( v  ) KoColorSpaceMaths< float, _channel_type_>::scaleToA( v )

template<typename _channel_type_>
void clamp(float* r, float* g, float* b);

#define FLOAT_CLAMP( v ) * v = (*v < 0.0) ? 0.0 : ( (*v>1.0) ? 1.0 : *v )

template<>
void clamp<quint8>(float* r, float* g, float* b)
{
    FLOAT_CLAMP(r);
    FLOAT_CLAMP(g);
    FLOAT_CLAMP(b);
}

template<>
void clamp<quint16>(float* r, float* g, float* b)
{
    FLOAT_CLAMP(r);
    FLOAT_CLAMP(g);
    FLOAT_CLAMP(b);
}

#ifdef HAVE_OPENEXR
template<>
void clamp<half>(float* r, float* g, float* b)
{
    Q_UNUSED(r);
    Q_UNUSED(g);
    Q_UNUSED(b);
}
#endif

template<>
void clamp<float>(float* r, float* g, float* b)
{
    Q_UNUSED(r);
    Q_UNUSED(g);
    Q_UNUSED(b);
}

template<typename _channel_type_>
class KisHSVAdjustment : public KoColorTransformation
{
    typedef KoRgbTraits<_channel_type_> RGBTrait;
    typedef typename RGBTrait::Pixel RGBPixel;
public:
    KisHSVAdjustment() {
    }

public:
    void transform(const quint8 *srcU8, quint8 *dstU8, qint32 nPixels) const {
        const RGBPixel* src = reinterpret_cast<const RGBPixel*>(srcU8);
        RGBPixel* dst = reinterpret_cast<RGBPixel*>(dstU8);
        float h, s, v, r, g, b;
        while (nPixels > 0) {
            RGBToHSV(SCALE_TO_FLOAT(src->red), SCALE_TO_FLOAT(src->green), SCALE_TO_FLOAT(src->blue), &h, &s, &v);
            h += m_adj_h;
            if (h > 360) h -= 360;
            if (h < 0) h += 360;
            s += m_adj_s;
            v += m_adj_v;
            HSVToRGB(h, s, v, &r, &g, &b);
            clamp< _channel_type_ >(&r, &g, &b);
            dst->red = SCALE_FROM_FLOAT(r);
            dst->green = SCALE_FROM_FLOAT(g);
            dst->blue = SCALE_FROM_FLOAT(b);
            dst->alpha = src->alpha;

            --nPixels;
            ++src;
            ++dst;
        }
    }

    /**
    * name - "h", "s" or "v"
    * (h)ue in range <-1.0, 1.0> ( for user, show as -180, 180)
    * (s)aturation in range <-1.0, 1.0> ( for user, show -100, 100)
    * (v)alue in range <-1.0, 1.0> (for user, show -100, 100)
    */
    virtual void setParameter(const QString& name, const QVariant& parameter) {
        if (name == "h") {
            m_adj_h = parameter.toDouble() * 180;
        } else if (name == "s") {
            m_adj_s = parameter.toDouble();
        } else if (name == "v") {
            m_adj_v = parameter.toDouble();
        }
    }

private:
    double m_adj_h, m_adj_s, m_adj_v;
};


KisHSVAdjustmentFactory::KisHSVAdjustmentFactory() : KoColorTransformationFactory("hsv_adjustment", i18n("HSV Adjustment"))
{

}

QList< QPair< KoID, KoID > > KisHSVAdjustmentFactory::supportedModels() const
{
    QList< QPair< KoID, KoID > > l;
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Integer8BitsColorDepthID));
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Integer16BitsColorDepthID));
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Float16BitsColorDepthID));
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Float32BitsColorDepthID));
    return l;
}

KoColorTransformation* KisHSVAdjustmentFactory::createTransformation(const KoColorSpace* colorSpace, QHash<QString, QVariant> parameters) const
{
    KoColorTransformation * adj;
    if (colorSpace->colorModelId() != RGBAColorModelID) {
        kError() << "Unsupported color space " << colorSpace->id() << " in KisHSVAdjustmentFactory::createTransformation";
        return 0;
    }
    if (colorSpace->colorDepthId() == Integer8BitsColorDepthID) {
        adj = new KisHSVAdjustment< quint8 >();
    } else if (colorSpace->colorDepthId() == Integer16BitsColorDepthID) {
        adj = new KisHSVAdjustment< quint16 >();
    }
#ifdef HAVE_OPENEXR
    else if (colorSpace->colorDepthId() == Float16BitsColorDepthID) {
        adj = new KisHSVAdjustment< half >();
    }
#endif
    else if (colorSpace->colorDepthId() == Float32BitsColorDepthID) {
        adj = new KisHSVAdjustment< float >();
    } else {
        kError() << "Unsupported color space " << colorSpace->id() << " in KisHSVAdjustmentFactory::createTransformation";
        return 0;
    }
    adj->setParameters(parameters);
    return adj;

}
