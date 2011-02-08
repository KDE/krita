/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
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

#ifndef _KOCOMPOSITEOPS_H_
#define _KOCOMPOSITEOPS_H_

#include <boost/type_traits.hpp>

#include <KoColorSpace.h>
#include <KoColorSpaceMaths.h>
#include <KoColorSpaceTraits.h>

#include "compositeops/KoCompositeOpGeneric.h"
#include "compositeops/KoCompositeOpCopyChannel.h"
#include "compositeops/KoCompositeOpAlphaDarken.h"
#include "compositeops/KoCompositeOpErase.h"
#include "compositeops/KoCompositeOpCopy2.h"
#include "compositeops/KoCompositeOpAtop.h"
// #include "compositeops/KoCompositeOpAdd.h"
// #include "compositeops/KoCompositeOpBurn.h"
// #include "compositeops/KoCompositeOpDivide.h"
// #include "compositeops/KoCompositeOpDodge.h"
// #include "compositeops/KoCompositeOpMultiply.h"
// #include "compositeops/KoCompositeOpOver.h"
// #include "compositeops/KoCompositeOpOverlay.h"
// #include "compositeops/KoCompositeOpScreen.h"
// #include "compositeops/KoCompositeOpSubtract.h"
// #include "compositeops/KoCompositeOpInversedSubtract.h"
// #include "compositeops/KoCompositeOpSoftlight.h"
// #include "compositeops/KoCompositeOpHardlight.h"


namespace Private {

template<class Traits, bool flag>
struct AddGeneralOps
{
    static void add(KoColorSpace* cs) { Q_UNUSED(cs); }
};

template<class Traits>
struct AddGeneralOps<Traits, true>
{
    typedef typename Traits::channels_type Arg;
    
    template<Arg compositeFunc(Arg, Arg)>
    static void add(KoColorSpace* cs, const QString& id, const QString& description, const QString& category, bool userVisible=true) {
        cs->addCompositeOp(new KoCompositeOpGenericSC<Traits, compositeFunc>(cs, id, description, category, userVisible));
    }
    
    static void add(KoColorSpace* cs) {
        cs->addCompositeOp(new KoCompositeOpOver<Traits>(cs));
        cs->addCompositeOp(new KoCompositeOpAlphaDarken<Traits>(cs));
        cs->addCompositeOp(new KoCompositeOpCopy2<Traits>(cs));
        cs->addCompositeOp(new KoCompositeOpErase<Traits>(cs));
        cs->addCompositeOp(new KoCompositeOpAtop<Traits>(cs, KoCompositeOp::categoryMix()));
        
        add<&cfGrainMerge>  (cs, COMPOSITE_GRAIN_MERGE  , i18n("Grain Merge")  , KoCompositeOp::categoryMix());
        add<&cfGrainExtract>(cs, COMPOSITE_GRAIN_EXTRACT, i18n("Grain Extract"), KoCompositeOp::categoryMix());
        add<&cfHardMix>     (cs, COMPOSITE_HARD_MIX     , i18n("Hard Mix")     , KoCompositeOp::categoryMix());
        
        add<&cfColorBurn>  (cs, COMPOSITE_BURN        , i18n("Color Burn")  , KoCompositeOp::categoryLight());
        add<&cfColorDodge> (cs, COMPOSITE_DODGE       , i18n("Color Dodge") , KoCompositeOp::categoryLight());
        add<&cfLinearBurn> (cs, COMPOSITE_LINEAR_BURN , i18n("Linear Burn") , KoCompositeOp::categoryLight());
        add<&cfAddition>   (cs, COMPOSITE_LINEAR_DODGE, i18n("Linear Dodge"), KoCompositeOp::categoryLight());
        add<&cfDarkenOnly> (cs, COMPOSITE_DARKEN      , i18n("Darken")      , KoCompositeOp::categoryLight());
        add<&cfLightenOnly>(cs, COMPOSITE_LIGHTEN     , i18n("Lighten")     , KoCompositeOp::categoryLight());
        add<&cfHardLight>  (cs, COMPOSITE_HARD_LIGHT  , i18n("Hard Light")  , KoCompositeOp::categoryLight());
        add<&cfSoftLight>  (cs, COMPOSITE_SOFT_LIGHT  , i18n("Soft Light")  , KoCompositeOp::categoryLight());
        add<&cfGammaLight> (cs, COMPOSITE_GAMMA_LIGHT , i18n("Gamma Light") , KoCompositeOp::categoryLight());
        add<&cfGammaDark>  (cs, COMPOSITE_GAMMA_DARK  , i18n("Gamma Dark")  , KoCompositeOp::categoryLight());
        add<&cfVividLight> (cs, COMPOSITE_VIVID_LIGHT , i18n("Vivid Light") , KoCompositeOp::categoryLight());
        add<&cfPinLight>   (cs, COMPOSITE_PIN_LIGHT   , i18n("Pin Light")   , KoCompositeOp::categoryLight());
        add<&cfLinearLight>(cs, COMPOSITE_LINEAR_LIGHT, i18n("Linear Light"), KoCompositeOp::categoryLight());
        
        add<&cfAddition>  (cs, COMPOSITE_ADD      , i18n("Addition")  , KoCompositeOp::categoryArithmetic());
        add<&cfSubtract>  (cs, COMPOSITE_SUBTRACT , i18n("Subtract")  , KoCompositeOp::categoryArithmetic());
        add<&cfDifference>(cs, COMPOSITE_DIFF     , i18n("Difference"), KoCompositeOp::categoryArithmetic());
        add<&cfMultiply>  (cs, COMPOSITE_MULT     , i18n("Multiply")  , KoCompositeOp::categoryArithmetic());
        add<&cfDivide>    (cs, COMPOSITE_DIVIDE   , i18n("Divide")    , KoCompositeOp::categoryArithmetic());
        add<&cfExclusion> (cs, COMPOSITE_EXCLUSION, i18n("Exclusion") , KoCompositeOp::categoryArithmetic());
        
        add<&cfScreen> (cs, COMPOSITE_SCREEN , i18n("Screen") , KoCompositeOp::categoryColor());
        add<&cfOverlay>(cs, COMPOSITE_OVERLAY, i18n("Overlay"), KoCompositeOp::categoryColor());
        
        add<&cfArcTangent>          (cs, COMPOSITE_ARC_TANGENT          , i18n("Arcus Tangent")        , KoCompositeOp::categoryMisc());
        add<&cfGeometricMean>       (cs, COMPOSITE_GEOMETRIC_MEAN       , i18n("Geometric Mean")       , KoCompositeOp::categoryMisc());
        add<&cfAllanon>             (cs, COMPOSITE_ALLANON              , i18n("Allanon")              , KoCompositeOp::categoryMisc());
        add<&cfParallel>            (cs, COMPOSITE_PARALLEL             , i18n("Parallel")             , KoCompositeOp::categoryMisc());
        add<&cfEquivalence>         (cs, COMPOSITE_EQUIVALENCE          , i18n("Equivalence")          , KoCompositeOp::categoryMisc());
        add<&cfAdditiveSubstractive>(cs, COMPOSITE_ADDITIVE_SUBSTRACTIVE, i18n("Additive-Substractive"), KoCompositeOp::categoryMisc());
    }
};

template<class Traits, bool flag>
struct AddRGBOps
{
    static void add(KoColorSpace* cs) { Q_UNUSED(cs); }
};

template<class Traits>
struct AddRGBOps<Traits, true>
{
    typedef float Arg;
    
    static const qint32 red_pos   = Traits::red_pos;
    static const qint32 green_pos = Traits::green_pos;
    static const qint32 blue_pos  = Traits::blue_pos;
    static const qint32 alpha_pos = Traits::alpha_pos;
    
    template<void compositeFunc(Arg, Arg, Arg, Arg&, Arg&, Arg&)>
    static void add(KoColorSpace* cs, const QString& id, const QString& description, const QString& category, bool userVisible=true) {
        cs->addCompositeOp(new KoCompositeOpGenericHSL<Traits, compositeFunc>(cs, id, description, category, userVisible));
    }
    
    static void add(KoColorSpace* cs) {
        cs->addCompositeOp(new KoCompositeOpCopyChannel<Traits,red_pos  >(cs, COMPOSITE_COPY_RED  , i18n("Copy Red")  , KoCompositeOp::categoryMisc()));
        cs->addCompositeOp(new KoCompositeOpCopyChannel<Traits,green_pos>(cs, COMPOSITE_COPY_GREEN, i18n("Copy Green"), KoCompositeOp::categoryMisc()));
        cs->addCompositeOp(new KoCompositeOpCopyChannel<Traits,blue_pos >(cs, COMPOSITE_COPY_BLUE , i18n("Copy Blue") , KoCompositeOp::categoryMisc()));
        
        if(alpha_pos != -1)
            cs->addCompositeOp(new KoCompositeOpCopyChannel<Traits,alpha_pos>(cs, COMPOSITE_COPY_OPACITY, i18n("Copy Alpha") , KoCompositeOp::categoryMisc()));
        
        add<&cfColor     <HSYType,Arg> >(cs, COMPOSITE_COLOR     , i18n("Color HSY")     , KoCompositeOp::categoryColor());
        add<&cfHue       <HSYType,Arg> >(cs, COMPOSITE_HUE       , i18n("Hue HSY")       , KoCompositeOp::categoryColor());
        add<&cfSaturation<HSYType,Arg> >(cs, COMPOSITE_SATURATION, i18n("Saturation HSY"), KoCompositeOp::categoryColor());
        add<&cfLightness <HSYType,Arg> >(cs, COMPOSITE_LUMINIZE  , i18n("Luminosity")    , KoCompositeOp::categoryColor());
        
        add<&cfColor     <HSLType,Arg> >(cs, COMPOSITE_COLOR_HSL     , i18n("Color HSL")     , KoCompositeOp::categoryColor());
        add<&cfHue       <HSLType,Arg> >(cs, COMPOSITE_HUE_HSL       , i18n("Hue HSL")       , KoCompositeOp::categoryColor());
        add<&cfSaturation<HSLType,Arg> >(cs, COMPOSITE_SATURATION_HSL, i18n("Saturation HSL"), KoCompositeOp::categoryColor());
        add<&cfLightness <HSLType,Arg> >(cs, COMPOSITE_LIGHTNESS     , i18n("Lightness")     , KoCompositeOp::categoryColor());
        
        add<&cfColor     <HSVType,Arg> >(cs, COMPOSITE_COLOR_HSV     , i18n("Color HSV")     , KoCompositeOp::categoryColor());
        add<&cfHue       <HSVType,Arg> >(cs, COMPOSITE_HUE_HSV       , i18n("Hue HSV")       , KoCompositeOp::categoryColor());
        add<&cfSaturation<HSVType,Arg> >(cs, COMPOSITE_SATURATION_HSV, i18n("Saturation HSV"), KoCompositeOp::categoryColor());
        add<&cfLightness <HSVType,Arg> >(cs, COMPOSITE_VALUE         , i18n("Value")         , KoCompositeOp::categoryColor());
        
        add<&cfColor     <HSIType,Arg> >(cs, COMPOSITE_COLOR_HSI     , i18n("Color HSI")     , KoCompositeOp::categoryColor());
        add<&cfHue       <HSIType,Arg> >(cs, COMPOSITE_HUE_HSI       , i18n("Hue HSI")       , KoCompositeOp::categoryColor());
        add<&cfSaturation<HSIType,Arg> >(cs, COMPOSITE_SATURATION_HSI, i18n("Saturation HSI"), KoCompositeOp::categoryColor());
        add<&cfLightness <HSIType,Arg> >(cs, COMPOSITE_INTENSITY     , i18n("Intensity")     , KoCompositeOp::categoryColor());
    }
};

}

/**
 * This function add to the colorspace all the composite ops defined by
 * the pigment library.
 */
template<class _Traits_>
void addStandardCompositeOps(KoColorSpace* cs)
{
    //cs->addCompositeOp(new KoCompositeOpAlphaDarken<_Traits_>(cs));
    //cs->addCompositeOp(new KoCompositeOpCopy2<_Traits_>(cs));
    //cs->addCompositeOp(new KoCompositeOpErase<_Traits_>(cs));
    //cs->addCompositeOp(new KoCompositeOpAdd<_Traits_>(cs));
    //cs->addCompositeOp(new KoCompositeOpAlphaDarken<_Traits_>(cs));
    //cs->addCompositeOp(new KoCompositeOpBurn<_Traits_>(cs));
    //cs->addCompositeOp(new KoCompositeOpCopy2<_Traits_>(cs));
    //cs->addCompositeOp(new KoCompositeOpDivide<_Traits_>(cs));
    //cs->addCompositeOp(new KoCompositeOpDodge<_Traits_>(cs));
    //cs->addCompositeOp(new KoCompositeOpErase<_Traits_>(cs));
    //cs->addCompositeOp(new KoCompositeOpMultiply<_Traits_>(cs));
    //cs->addCompositeOp(new KoCompositeOpOver<_Traits_>(cs));
    //cs->addCompositeOp(new KoCompositeOpOverlay<_Traits_>(cs));
    //cs->addCompositeOp(new KoCompositeOpScreen<_Traits_>(cs));
    //cs->addCompositeOp(new KoCompositeOpSubtract<_Traits_>(cs));
    //cs->addCompositeOp(new KoCompositeOpSoftlight<_Traits_>(cs));
    //cs->addCompositeOp(new KoCompositeOpHardlight<_Traits_>(cs));
    
    typedef typename _Traits_::channels_type channels_type;
    
    static const bool useGeneralOps = true;
    static const bool useHSLOps     = boost::is_base_of<KoRgbTraits<channels_type>, _Traits_>::value;
    
    Private::AddGeneralOps<_Traits_, useGeneralOps>::add(cs);
    Private::AddRGBOps    <_Traits_, useHSLOps    >::add(cs);
}

#endif
