/*
 *  SPDX-FileCopyrightText: 2005 Adrian Page <adrian@pagenet.plus.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "KoCompositeOp.h"

#include <klocalizedstring.h>
#include <KoID.h>
#include <QList>

#include "KoColorSpace.h"
#include "KoColorSpaceMaths.h"
#include "KoCompositeOpRegistry.h"

static QString compositeOpDisplayName(const QString &id)
{
    return KoCompositeOpRegistry::instance().getCompositeOpDisplayName(id);
}

static QString categoryDisplayName(const QString &id)
{
    return KoCompositeOpRegistry::instance().getCategoryDisplayName(id);
}

#define LAZY_STATIC_CATEGORY_DISPLAY_NAME(n) \
    []() { \
        static const QString name = categoryDisplayName(QStringLiteral(n)); \
        return name; \
    }()

QString KoCompositeOp::categoryArithmetic() { return LAZY_STATIC_CATEGORY_DISPLAY_NAME("arithmetic"); }
QString KoCompositeOp::categoryBinary()     { return LAZY_STATIC_CATEGORY_DISPLAY_NAME("binary");     }
QString KoCompositeOp::categoryModulo()     { return LAZY_STATIC_CATEGORY_DISPLAY_NAME("modulo");     }
QString KoCompositeOp::categoryNegative()   { return LAZY_STATIC_CATEGORY_DISPLAY_NAME("negative");   }
QString KoCompositeOp::categoryLight()      { return LAZY_STATIC_CATEGORY_DISPLAY_NAME("light");      }
QString KoCompositeOp::categoryDark()       { return LAZY_STATIC_CATEGORY_DISPLAY_NAME("dark");       }
QString KoCompositeOp::categoryHSY()        { return LAZY_STATIC_CATEGORY_DISPLAY_NAME("hsy");        }
QString KoCompositeOp::categoryHSI()        { return LAZY_STATIC_CATEGORY_DISPLAY_NAME("hsi");        }
QString KoCompositeOp::categoryHSL()        { return LAZY_STATIC_CATEGORY_DISPLAY_NAME("hsl");        }
QString KoCompositeOp::categoryHSV()        { return LAZY_STATIC_CATEGORY_DISPLAY_NAME("hsv");        }
QString KoCompositeOp::categoryMix()        { return LAZY_STATIC_CATEGORY_DISPLAY_NAME("mix");        }
QString KoCompositeOp::categoryMisc()       { return LAZY_STATIC_CATEGORY_DISPLAY_NAME("misc");       }
QString KoCompositeOp::categoryQuadratic()  { return LAZY_STATIC_CATEGORY_DISPLAY_NAME("quadratic");  }

KoCompositeOp::ParameterInfo::ParameterInfo()
    : opacity(1.0f)
    , flow(1.0f)
    , lastOpacity(&opacity)
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

KoCompositeOp::KoCompositeOp(const KoColorSpace * cs, const QString& id, const QString & category)
        : d(new Private)
{
    d->colorSpace = cs;
    d->id = id;
    d->description = compositeOpDisplayName(id);
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
