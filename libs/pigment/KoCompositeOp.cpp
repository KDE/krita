/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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

#include "KoCompositeOp.h"

#include <klocalizedstring.h>
#include <KoID.h>
#include <QList>

#include "KoColorSpace.h"
#include "KoColorSpaceMaths.h"

QString KoCompositeOp::categoryColor()
{
    return i18n("Color");
}

QString KoCompositeOp::categoryArithmetic() { return i18n("Arithmetic"); }
QString KoCompositeOp::categoryBinary()     { return i18n("Binary");     }
QString KoCompositeOp::categoryModulo()     { return i18n("Modulo");     }
QString KoCompositeOp::categoryNegative()   { return i18n("Negative");   }
QString KoCompositeOp::categoryLight()      { return i18n("Lighten");    }
QString KoCompositeOp::categoryDark()       { return i18n("Darken");     }
QString KoCompositeOp::categoryHSY()        { return i18n("HSY");        }
QString KoCompositeOp::categoryHSI()        { return i18n("HSI");        }
QString KoCompositeOp::categoryHSL()        { return i18n("HSL");        }
QString KoCompositeOp::categoryHSV()        { return i18n("HSV");        }
QString KoCompositeOp::categoryMix()        { return i18n("Mix");        }
QString KoCompositeOp::categoryMisc()       { return i18n("Misc");       }
QString KoCompositeOp::categoryQuadratic()  { return i18n("Quadratic");  }

KoCompositeOp::ParameterInfo::ParameterInfo()
    : opacity(1.0f),
      flow(1.0f),
      lastOpacity(&opacity)
{
}

KoCompositeOp::ParameterInfo::ParameterInfo(const ParameterInfo &rhs)
{
    copy(rhs);
}

KoCompositeOp::ParameterInfo& KoCompositeOp::ParameterInfo::operator=(const ParameterInfo &rhs)
{
    copy(rhs);
    return *this;
}

void KoCompositeOp::ParameterInfo::setOpacityAndAverage(float _opacity, float _averageOpacity)
{
    if (qFuzzyCompare(_opacity, _averageOpacity)) {
        opacity = _opacity;
        lastOpacity = &opacity;
    } else {
        opacity = _opacity;
        _lastOpacityData = _averageOpacity;
        lastOpacity = &_lastOpacityData;
    }
}

void KoCompositeOp::ParameterInfo::copy(const ParameterInfo &rhs)
{
    dstRowStart = rhs.dstRowStart;
    dstRowStride = rhs.dstRowStride;
    srcRowStart = rhs.srcRowStart;
    srcRowStride = rhs.srcRowStride;
    maskRowStart = rhs.maskRowStart;
    maskRowStride = rhs.maskRowStride;
    rows = rhs.rows;
    cols = rhs.cols;
    opacity = rhs.opacity;
    flow = rhs.flow;
    _lastOpacityData = rhs._lastOpacityData;
    channelFlags = rhs.channelFlags;

    lastOpacity = rhs.lastOpacity == &rhs.opacity ?
        &opacity : &_lastOpacityData;
}

void KoCompositeOp::ParameterInfo::updateOpacityAndAverage(float value) {
    const float exponent = 0.1;

    opacity = value;

    if (*lastOpacity < opacity) {
        lastOpacity = &opacity;
    } else {
        _lastOpacityData = exponent * opacity + (1.0 - exponent) * (*lastOpacity);
        lastOpacity = &_lastOpacityData;
    }
}

struct Q_DECL_HIDDEN KoCompositeOp::Private {
    const KoColorSpace * colorSpace;
    QString id;
    QString description;
    QString category;
    QBitArray defaultChannelFlags;
};

KoCompositeOp::KoCompositeOp() : d(new Private)
{

}

KoCompositeOp::~KoCompositeOp()
{
    delete d;
}

KoCompositeOp::KoCompositeOp(const KoColorSpace * cs, const QString& id,  const QString& description, const QString & category)
        : d(new Private)
{
    d->colorSpace = cs;
    d->id = id;
    d->description = description;
    d->category = category;
    if (d->category.isEmpty()) {
        d->category = categoryMisc();
    }
}

void KoCompositeOp::composite(quint8 *dstRowStart, qint32 dstRowStride,
                              const quint8 *srcRowStart, qint32 srcRowStride,
                              const quint8 *maskRowStart, qint32 maskRowStride,
                              qint32 rows, qint32 numColumns,
                              quint8 opacity, const QBitArray& channelFlags) const
{
    KoCompositeOp::ParameterInfo params;
    params.dstRowStart   = dstRowStart;
    params.dstRowStride  = dstRowStride;
    params.srcRowStart   = srcRowStart;
    params.srcRowStride  = srcRowStride;
    params.maskRowStart  = maskRowStart;
    params.maskRowStride = maskRowStride;
    params.rows          = rows;
    params.cols          = numColumns;
    params.opacity       = float(opacity) / 255.0f;
    params.flow          = 1.0f;
    params.channelFlags  = channelFlags;
    composite(params);
}

void KoCompositeOp::composite(const KoCompositeOp::ParameterInfo& params) const
{
    using namespace Arithmetic;

    composite(params.dstRowStart           , params.dstRowStride ,
              params.srcRowStart           , params.srcRowStride ,
              params.maskRowStart          , params.maskRowStride,
              params.rows                  , params.cols         ,
              scale<quint8>(params.opacity), params.channelFlags );
}


QString KoCompositeOp::category() const
{
    return d->category;
}

QString KoCompositeOp::id() const
{
    return d->id;
}

QString KoCompositeOp::description() const
{
    return d->description;
}

const KoColorSpace * KoCompositeOp::colorSpace() const
{
    return d->colorSpace;
}
