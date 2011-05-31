/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
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

#include <kglobal.h>
#include <klocale.h>
#include <KoID.h>
#include <QList>

#include "KoCompositeOp.h"
#include "KoColorSpace.h"

KoCompositeOpRegistry::KoCompositeOpRegistry()
{
    m_categories
        << KoID("arithmetic", i18n("Arithmetic"))
        << KoID("dark"      , i18n("Darken"))
        << KoID("light"     , i18n("Lighten"))
        << KoID("negative"  , i18n("Negative"))
        << KoID("mix"       , i18n("Mix"))
        << KoID("misc"      , i18n("Misc"))
        << KoID("hsy"       , i18n("HSY"))
        << KoID("hsi"       , i18n("HSI"))
        << KoID("hsl"       , i18n("HSL"))
        << KoID("hsv"       , i18n("HSV"));
    
    m_categoryMap[m_categories[0].id()] // Arithmetic
        << KoID(COMPOSITE_ADD             , i18n("Addition"))
        << KoID(COMPOSITE_SUBTRACT        , i18n("Substract"))
        << KoID(COMPOSITE_MULT            , i18n("Multiply"))
        << KoID(COMPOSITE_DIVIDE          , i18n("Divide"))
        << KoID(COMPOSITE_INVERSE_SUBTRACT, i18n("Inverse Substract"));
    
    m_categoryMap[m_categories[1].id()] // Darken
        << KoID(COMPOSITE_BURN       , i18n("Burn"))
        << KoID(COMPOSITE_LINEAR_BURN, i18n("Linear Burn"))
        << KoID(COMPOSITE_DARKEN     , i18n("Darken"))
        << KoID(COMPOSITE_GAMMA_DARK , i18n("Gamma Dark"));
        
    m_categoryMap[m_categories[2].id()] // Lighten
        << KoID(COMPOSITE_DODGE       , i18n("Color Dodge"))
        << KoID(COMPOSITE_LINEAR_DODGE, i18n("Linear Dodge"))
        << KoID(COMPOSITE_LIGHTEN     , i18n("Lighten"))
        << KoID(COMPOSITE_LINEAR_LIGHT, i18n("Linear Light"))
        << KoID(COMPOSITE_SCREEN      , i18n("Screen"))
        << KoID(COMPOSITE_PIN_LIGHT   , i18n("Pin Light"))
        << KoID(COMPOSITE_VIVID_LIGHT , i18n("Vivid Light"))
        << KoID(COMPOSITE_HARD_LIGHT  , i18n("Hard Light"))
        << KoID(COMPOSITE_SOFT_LIGHT  , i18n("Soft Light"))
        << KoID(COMPOSITE_GAMMA_LIGHT , i18n("Gamma Light"));
    
    m_categoryMap[m_categories[3].id()] // Negative
        << KoID(COMPOSITE_DIFF                 , i18n("Difference"))
        << KoID(COMPOSITE_EQUIVALENCE          , i18n("Equivalence"))
        << KoID(COMPOSITE_ADDITIVE_SUBSTRACTIVE, i18n("Additive Substractive"))
        << KoID(COMPOSITE_EXCLUSION            , i18n("Exclusion"))
        << KoID(COMPOSITE_ARC_TANGENT          , i18n("Arcus Tangent"));
    
    m_categoryMap[m_categories[4].id()] // Mix
        << KoID(COMPOSITE_OVER          , i18n("Normal"))
        << KoID(COMPOSITE_OVERLAY       , i18n("Overlay"))
        << KoID(COMPOSITE_ERASE         , i18n("Erase"))
        << KoID(COMPOSITE_ALPHA_DARKEN  , i18n("Alpha Darken"))
        << KoID(COMPOSITE_HARD_MIX      , i18n("Hard Mix"))
        << KoID(COMPOSITE_GRAIN_MERGE   , i18n("Grain Merge"))
        << KoID(COMPOSITE_GRAIN_EXTRACT , i18n("Grain Extract"))
        << KoID(COMPOSITE_PARALLEL      , i18n("Parallel"))
        << KoID(COMPOSITE_ALLANON       , i18n("Allanon"))
        << KoID(COMPOSITE_GEOMETRIC_MEAN, i18n("Geometric Mean"));
    
    m_categoryMap[m_categories[5].id()] // Misc
        << KoID(COMPOSITE_BUMPMAP   , i18n("Bumpmap"))
        << KoID(COMPOSITE_DISSOLVE  , i18n("Dissolve"))
        << KoID(COMPOSITE_COPY_RED  , i18n("Copy Red"))
        << KoID(COMPOSITE_COPY_GREEN, i18n("Copy Green"))
        << KoID(COMPOSITE_COPY_BLUE , i18n("Copy Blue"));
        
    m_categoryMap[m_categories[6].id()] // HSY
        << KoID(COMPOSITE_COLOR         , i18n("Color"))
        << KoID(COMPOSITE_HUE           , i18n("Hue"))
        << KoID(COMPOSITE_SATURATION    , i18n("Saturation"))
        << KoID(COMPOSITE_LUMINIZE      , i18n("Luminosity"))
        << KoID(COMPOSITE_DEC_SATURATION, i18n("Decrease Saturation"))
        << KoID(COMPOSITE_INC_SATURATION, i18n("Increase Saturation"))
        << KoID(COMPOSITE_DEC_LUMINOSITY, i18n("Decrease Luminosity"))
        << KoID(COMPOSITE_INC_LUMINOSITY, i18n("Increase Luminosity"));
    
    m_categoryMap[m_categories[7].id()] // HSI
        << KoID(COMPOSITE_COLOR_HSI         , i18n("Color HSI"))
        << KoID(COMPOSITE_HUE_HSI           , i18n("Hue HSI"))
        << KoID(COMPOSITE_SATURATION_HSI    , i18n("Saturation HSI"))
        << KoID(COMPOSITE_INTENSITY         , i18n("Intensity"))
        << KoID(COMPOSITE_DEC_SATURATION_HSI, i18n("Decrease Saturation HSI"))
        << KoID(COMPOSITE_INC_SATURATION_HSI, i18n("Increase Saturation HSI"))
        << KoID(COMPOSITE_DEC_INTENSITY     , i18n("Decrease Intensity"))
        << KoID(COMPOSITE_INC_INTENSITY     , i18n("Increase Intensity"));
    
    m_categoryMap[m_categories[8].id()] // HSL
        << KoID(COMPOSITE_COLOR_HSL         , i18n("Color HSL"))
        << KoID(COMPOSITE_HUE_HSL           , i18n("Hue HSL"))
        << KoID(COMPOSITE_SATURATION_HSL    , i18n("Saturation HSL"))
        << KoID(COMPOSITE_LIGHTNESS         , i18n("Lightness"))
        << KoID(COMPOSITE_DEC_SATURATION_HSL, i18n("Decrease Saturation HSL"))
        << KoID(COMPOSITE_INC_SATURATION_HSL, i18n("Increase Saturation HSL"))
        << KoID(COMPOSITE_DEC_LIGHTNESS     , i18n("Decrease Lightness"))
        << KoID(COMPOSITE_INC_LIGHTNESS     , i18n("Increase Lightness"));
    
    m_categoryMap[m_categories[9].id()] // HSV
        << KoID(COMPOSITE_COLOR_HSV         , i18n("Color HSV"))
        << KoID(COMPOSITE_HUE_HSV           , i18n("Hue HSV"))
        << KoID(COMPOSITE_SATURATION_HSV    , i18n("Saturation HSV"))
        << KoID(COMPOSITE_VALUE             , i18n("Lightness"))
        << KoID(COMPOSITE_DEC_SATURATION_HSV, i18n("Decrease Saturation HSV"))
        << KoID(COMPOSITE_INC_SATURATION_HSV, i18n("Increase Saturation HSV"))
        << KoID(COMPOSITE_DEC_VALUE         , i18n("Decrease Value"))
        << KoID(COMPOSITE_INC_VALUE         , i18n("Increase Value"));
}

const KoCompositeOpRegistry& KoCompositeOpRegistry::instance()
{
    K_GLOBAL_STATIC(KoCompositeOpRegistry, registry);
    return *registry;
}

KoCompositeOpRegistry::KoIDList KoCompositeOpRegistry::getCompositeOps(const QString& categoryID) const
{
    if(m_categoryMap.contains(categoryID))
        return m_categoryMap[categoryID];
    
    return KoIDList();
}

KoCompositeOpRegistry::KoIDList KoCompositeOpRegistry::getCategories() const
{
    return m_categories;
}


QString KoCompositeOp::categoryColor()
{
    return i18n("Color");
}

QString KoCompositeOp::categoryArithmetic() { return i18n("Arithmetic"); }
QString KoCompositeOp::categoryNegative()   { return i18n("Negative");   }
QString KoCompositeOp::categoryLight()      { return i18n("Lighten");    }
QString KoCompositeOp::categoryDark()       { return i18n("Darken");     }
QString KoCompositeOp::categoryHSY()        { return i18n("HSY");        }
QString KoCompositeOp::categoryHSI()        { return i18n("HSI");        }
QString KoCompositeOp::categoryHSL()        { return i18n("HSL");        }
QString KoCompositeOp::categoryHSV()        { return i18n("HSV");        }
QString KoCompositeOp::categoryMix()        { return i18n("Mix");        }
QString KoCompositeOp::categoryMisc()       { return i18n("Misc");       }

struct KoCompositeOp::Private {
    const KoColorSpace * colorSpace;
    QString id;
    QString description;
    QString category;
    bool userVisible;
    QBitArray defaultChannelFlags;
};

KoCompositeOp::KoCompositeOp() : d(new Private)
{

}

KoCompositeOp::~KoCompositeOp()
{
    delete d;
}

KoCompositeOp::KoCompositeOp(const KoColorSpace * cs, const QString& id,  const QString& description, const QString & category, const bool userVisible)
        : d(new Private)
{
    d->colorSpace = cs;
    d->id = id;
    d->description = description;
    d->userVisible = userVisible;
    d->category = category;
    if (d->category.isEmpty()) {
        d->category = categoryMisc();
    }
}

void KoCompositeOp::composite(quint8 *dstRowStart, qint32 dstRowStride,
                              const quint8 *srcRowStart, qint32 srcRowStride,
                              const quint8 *maskRowStart, qint32 maskRowStride,
                              qint32 rows, qint32 numColumns,
                              quint8 opacity) const
{
    composite(dstRowStart, dstRowStride,
              srcRowStart, srcRowStride,
              maskRowStart, maskRowStride,
              rows, numColumns,
              opacity, d->defaultChannelFlags);
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

bool KoCompositeOp::userVisible() const
{
    return d->userVisible;
}
