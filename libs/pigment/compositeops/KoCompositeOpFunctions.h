/*
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
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

#ifndef KOCOMPOSITEOP_FUNCTIONS_H_
#define KOCOMPOSITEOP_FUNCTIONS_H_

#include <KoColorSpaceMaths.h>

template<class HSXType, class TReal>
inline void cfReorientedNormalMapCombine(TReal srcR, TReal srcG, TReal srcB, TReal& dstR, TReal& dstG, TReal& dstB)
{
    // see http://blog.selfshadow.com/publications/blending-in-detail/ by Barre-Brisebois and Hill
    TReal tx = 2*srcR-1;
    TReal ty = 2*srcG-1;
    TReal tz = 2*srcB;
    TReal ux = -2*dstR+1;
    TReal uy = -2*dstG+1;
    TReal uz = 2*dstB-1;
    TReal k = (tx*ux+ty*uy+tz*uz)/tz; // dot(t,u)/t.z
    TReal rx = tx*k-ux;
    TReal ry = ty*k-uy;
    TReal rz = tz*k-uz;
    k = 1/sqrt(rx*rx+ry*ry+rz*rz); // normalize result
    rx *= k;
    ry *= k;
    rz *= k;
    dstR = rx*0.5+0.5;
    dstG = ry*0.5+0.5;
    dstB = rz*0.5+0.5;
}

template<class HSXType, class TReal>
inline void cfColor(TReal sr, TReal sg, TReal sb, TReal& dr, TReal& dg, TReal& db) {
    TReal lum = getLightness<HSXType>(dr, dg, db);
    dr = sr;
    dg = sg;
    db = sb;
    setLightness<HSXType>(dr, dg, db, lum);
}

template<class HSXType, class TReal>
inline void cfLightness(TReal sr, TReal sg, TReal sb, TReal& dr, TReal& dg, TReal& db) {
    setLightness<HSXType>(dr, dg, db, getLightness<HSXType>(sr, sg, sb));
}

template<class HSXType, class TReal>
inline void cfIncreaseLightness(TReal sr, TReal sg, TReal sb, TReal& dr, TReal& dg, TReal& db) {
    addLightness<HSXType>(dr, dg, db, getLightness<HSXType>(sr, sg, sb));
}

template<class HSXType, class TReal>
inline void cfDecreaseLightness(TReal sr, TReal sg, TReal sb, TReal& dr, TReal& dg, TReal& db) {
    addLightness<HSXType>(dr, dg, db, getLightness<HSXType>(sr, sg, sb) - TReal(1.0));
}

template<class HSXType, class TReal>
inline void cfSaturation(TReal sr, TReal sg, TReal sb, TReal& dr, TReal& dg, TReal& db) {
    TReal sat   = getSaturation<HSXType>(sr, sg, sb);
    TReal light = getLightness<HSXType>(dr, dg, db);
    setSaturation<HSXType>(dr, dg, db, sat);
    setLightness<HSXType>(dr, dg, db, light);
}

template<class HSXType, class TReal>
inline void cfIncreaseSaturation(TReal sr, TReal sg, TReal sb, TReal& dr, TReal& dg, TReal& db) {
    using namespace Arithmetic;
    TReal sat   = lerp(getSaturation<HSXType>(dr,dg,db), unitValue<TReal>(), getSaturation<HSXType>(sr,sg,sb));
    TReal light = getLightness<HSXType>(dr, dg, db);
    setSaturation<HSXType>(dr, dg, db, sat);
    setLightness<HSXType>(dr, dg, db, light);
}

template<class HSXType, class TReal>
inline void cfDecreaseSaturation(TReal sr, TReal sg, TReal sb, TReal& dr, TReal& dg, TReal& db) {
    using namespace Arithmetic;
    TReal sat   = lerp(zeroValue<TReal>(), getSaturation<HSXType>(dr,dg,db), getSaturation<HSXType>(sr,sg,sb));
    TReal light = getLightness<HSXType>(dr, dg, db);
    setSaturation<HSXType>(dr, dg, db, sat);
    setLightness<HSXType>(dr, dg, db, light);
}

template<class HSXType, class TReal>
inline void cfHue(TReal sr, TReal sg, TReal sb, TReal& dr, TReal& dg, TReal& db) {
    TReal sat = getSaturation<HSXType>(dr, dg, db);
    TReal lum = getLightness<HSXType>(dr, dg, db);
    dr = sr;
    dg = sg;
    db = sb;
    setSaturation<HSXType>(dr, dg, db, sat);
    setLightness<HSXType>(dr, dg, db, lum);
}

template<class HSXType, class TReal>
inline void cfTangentNormalmap(TReal sr, TReal sg, TReal sb, TReal& dr, TReal& dg, TReal& db) {
    using namespace Arithmetic;
    TReal half=halfValue<TReal>();
    
    dr = sr+(dr-half);
    dg = sg+(dg-half);
    db = sb+(db-unitValue<TReal>());
} 
    
template<class HSXType, class TReal>
inline void cfDarkerColor(TReal sr, TReal sg, TReal sb, TReal& dr, TReal& dg, TReal& db) {
    
    TReal lum = getLightness<HSXType>(dr, dg, db);
    TReal lum2 = getLightness<HSXType>(sr, sg, sb);
    if (lum<lum2) {
        sr = dr;
        sg = dg;
        sb = db;
    }
    else {
        dr = sr;
        dg = sg;
        db = sb;
    }

}

template<class HSXType, class TReal>
inline void cfLighterColor(TReal sr, TReal sg, TReal sb, TReal& dr, TReal& dg, TReal& db) {
    
    TReal lum = getLightness<HSXType>(dr, dg, db);
    TReal lum2 = getLightness<HSXType>(sr, sg, sb);
    if (lum>lum2) {
        sr = dr;
        sg = dg;
        sb = db;
    }
    else {
        dr = sr;
        dg = sg;
        db = sb;
    }
}

template<class T>
inline T cfColorBurn(T src, T dst) {
    using namespace Arithmetic;
    
    if(dst == unitValue<T>())
        return unitValue<T>();
    
    T invDst = inv(dst);
    
    if(src < invDst)
        return zeroValue<T>();
    
    return inv(clamp<T>(div(invDst, src)));
}

template<class T>
inline T cfLinearBurn(T src, T dst) {
    using namespace Arithmetic;
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    return clamp<T>(composite_type(src) + dst - unitValue<T>());
}

template<class T>
inline T cfColorDodge(T src, T dst) {
    using namespace Arithmetic;
    //Fixing Color Dodge to avoid ZX Colors on bright area.

    if(src == unitValue<T>())
        return unitValue<T>();
    
    T invSrc = inv(src);
    
    if(invSrc == zeroValue<T>())
        return unitValue<T>();
    
    return Arithmetic::clamp<T>(div(dst, invSrc));
}

template<class T>
inline T cfAddition(T src, T dst) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    return Arithmetic::clamp<T>(composite_type(src) + dst);
}

template<class T>
inline T cfSubtract(T src, T dst) {
    using namespace Arithmetic;
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    return clamp<T>(composite_type(dst) - src);
}

template<class T>
inline T cfInverseSubtract(T src, T dst) {
    using namespace Arithmetic;
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    return clamp<T>(composite_type(dst) - inv(src));
}

template<class T>
inline T cfExclusion(T src, T dst) {
    using namespace Arithmetic;
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    
    composite_type x = mul(src, dst);
    return clamp<T>(composite_type(dst) + src - (x + x));
}

template<class T>
inline T cfDivide(T src, T dst) {
    using namespace Arithmetic;
    //typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    
    if(src == zeroValue<T>())
        return (dst == zeroValue<T>()) ? zeroValue<T>() : unitValue<T>();
    
    return clamp<T>(div(dst, src));
}

template<class T>
inline T cfHardLight(T src, T dst) {
    using namespace Arithmetic;
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    
    composite_type src2 = composite_type(src) + src;
    
    if(src > halfValue<T>()) {
        // screen(src*2.0 - 1.0, dst)
        src2 -= unitValue<T>();

        // src2 is guaranteed to be smaller than unitValue<T>() now
        return Arithmetic::unionShapeOpacity(T(src2), dst);
    }
    
    // src2 is guaranteed to be smaller than unitValue<T>() due to 'if'
    return Arithmetic::mul(T(src2), dst);
}

template<class T>
inline T cfSoftLightSvg(T src, T dst) {
    using namespace Arithmetic;

    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);

    if(fsrc > 0.5f) {
        qreal D = (fdst > 0.25f) ? sqrt(fdst) : ((16.0f*fdst - 12.0)*fdst + 4.0f)*fdst;
        return scale<T>(fdst + (2.0f*fsrc - 1.0f) * (D - fdst));
    }

    return scale<T>(fdst - (1.0f - 2.0f * fsrc) * fdst * (1.0f - fdst));
}


template<class T>
inline T cfSoftLight(T src, T dst) {
    using namespace Arithmetic;
    
    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);
    
    if(fsrc > 0.5f) {
        return scale<T>(fdst + (2.0f * fsrc - 1.0f) * (sqrt(fdst) - fdst));
    }
    
    return scale<T>(fdst - (1.0f - 2.0f*fsrc) * fdst * (1.0f - fdst));
}

template<class T>
inline T cfVividLight(T src, T dst) {
    using namespace Arithmetic;
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    
    if(src < halfValue<T>()) {
        if(src == zeroValue<T>())
            return (dst == unitValue<T>()) ? unitValue<T>() : zeroValue<T>();

        // min(1,max(0,1-(1-dst) / (2*src)))
        composite_type src2 = composite_type(src) + src;
        composite_type dsti = inv(dst);
        return clamp<T>(unitValue<T>() - (dsti * unitValue<T>() / src2));
    }
    
    if(src == unitValue<T>())
        return (dst == zeroValue<T>()) ? zeroValue<T>() : unitValue<T>();
    
    // min(1,max(0, dst / (2*(1-src)))
    composite_type srci2 = inv(src);
    srci2 += srci2;
    return clamp<T>(composite_type(dst) * unitValue<T>() / srci2);
}

template<class T>
inline T cfPinLight(T src, T dst) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    // TODO: verify that the formula is correct (the first max would be useless here)
    // max(0, max(2*src-1, min(dst, 2*src)))
    composite_type src2 = composite_type(src) + src;
    composite_type a    = qMin<composite_type>(dst, src2);
    composite_type b    = qMax<composite_type>(src2-Arithmetic::unitValue<T>(), a);
    return T(b);
}

template<class T>
inline T cfArcTangent(T src, T dst) {
    using namespace Arithmetic;
    
    if(dst == zeroValue<T>())
        return (src == zeroValue<T>()) ? zeroValue<T>() : unitValue<T>();
    
    return scale<T>(2.0 * atan(scale<qreal>(src) / scale<qreal>(dst)) / Arithmetic::pi);
}

template<class T>
inline T cfAllanon(T src, T dst) {
    using namespace Arithmetic;
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    // (dst + src) / 2   [or (dst + src) * 0.5]
    return T((composite_type(src) + dst) * halfValue<T>() / unitValue<T>());
}

template<class T>
inline T cfLinearLight(T src, T dst) {
    using namespace Arithmetic;
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    
    // min(1,max(0,(dst + 2*src)-1))
    return clamp<T>((composite_type(src) + src + dst) - unitValue<T>());
}

template<class T>
inline T cfParallel(T src, T dst) {
    using namespace Arithmetic;
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    
    // min(max(2 / (1/dst + 1/src), 0), 1)
    composite_type unit = unitValue<T>();
    composite_type s    = (src != zeroValue<T>()) ? div<T>(unit, src) : unit;
    composite_type d    = (dst != zeroValue<T>()) ? div<T>(unit, dst) : unit;    
    if (src == zeroValue<T>()) {
        return zeroValue<T>();    
    }

    if (dst == zeroValue<T>()) {
        return zeroValue<T>();    
    }

    return clamp<T>((unit+unit) * unit / (d+s));
}

template<class T>
inline T cfEquivalence(T src, T dst) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    // 1 - abs(dst - src)
    composite_type x = composite_type(dst) - src;
    return (x < Arithmetic::zeroValue<T>()) ? T(-x) : T(x);
}

template<class T>
inline T cfGrainMerge(T src, T dst) {
    using namespace Arithmetic;
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    return clamp<T>(composite_type(dst) + src - halfValue<T>());
}

template<class T>
inline T cfGrainExtract(T src, T dst) {
    using namespace Arithmetic;
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    return clamp<T>(composite_type(dst) - src + halfValue<T>());
}

template<class T>
inline T cfHardMix(T src, T dst) {
    return (dst > Arithmetic::halfValue<T>()) ? cfColorDodge(src,dst) : cfColorBurn(src,dst);
}

template<class T>
inline T cfHardMixPhotoshop(T src, T dst) {
    using namespace Arithmetic;
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;

    const composite_type sum = composite_type(src) + dst;

    return sum > unitValue<T>() ? unitValue<T>() : zeroValue<T>();
}

template<class T>
inline T cfAdditiveSubtractive(T src, T dst) {
    using namespace Arithmetic;
    // min(1,max(0,abs(sqr(CB)-sqr(CT))))
    qreal x = sqrt(scale<qreal>(dst)) - sqrt(scale<qreal>(src));
    return scale<T>((x < 0.0) ? -x : x);
}

template<class T>
inline T cfGammaDark(T src, T dst) {
    using namespace Arithmetic;
    
    if(src == zeroValue<T>())
        return zeroValue<T>();
    
    // power(dst, 1/src)
    return scale<T>(pow(scale<qreal>(dst), 1.0/scale<qreal>(src)));
}

template<class T>
inline T cfGammaLight(T src, T dst) {
    using namespace Arithmetic;
    return scale<T>(pow(scale<qreal>(dst), scale<qreal>(src)));
}

template<class T>
inline T cfGammaIllumination(T src, T dst) {
    using namespace Arithmetic;
    return inv(cfGammaDark(inv(src),inv(dst)));
}

template<class T>
inline T cfGeometricMean(T src, T dst) {
    using namespace Arithmetic;
    return scale<T>(sqrt(scale<qreal>(dst) * scale<qreal>(src)));
}

template<class T>
inline T cfOver(T src, T dst) { Q_UNUSED(dst); return src; }

template<class T>
inline T cfOverlay(T src, T dst) { return cfHardLight(dst, src); }

template<class T>
inline T cfMultiply(T src, T dst) { return Arithmetic::mul(src, dst); }

template<class T>
inline T cfHardOverlay(T src, T dst) {
    using namespace Arithmetic;

    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);
    
    if (fsrc == 1.0) {
        return scale<T>(1.0);}

    if(fsrc > 0.5f) {
        return scale<T>(cfDivide(inv(2.0 * fsrc - 1.0f), fdst));
    }
    return scale<T>(mul(2.0 * fsrc, fdst));
}

template<class T>
inline T cfDifference(T src, T dst) { return qMax(src,dst) - qMin(src,dst); }

template<class T>
inline T cfScreen(T src, T dst) { return Arithmetic::unionShapeOpacity(src, dst); }

template<class T>
inline T cfDarkenOnly(T src, T dst) { return qMin(src, dst); }

template<class T>
inline T cfLightenOnly(T src, T dst) { return qMax(src, dst); }

template<class T>
inline T cfGlow(T src, T dst) {
    using namespace Arithmetic;
        // see http://www.pegtop.net/delphi/articles/blendmodes/quadratic.htm for formulas of Quadratic Blending Modes like Glow, Reflect, Freeze, and Heat

    if (dst == unitValue<T>()) {
        return unitValue<T>();
    }

    return clamp<T>(div(mul(src, src), inv(dst)));
}

template<class T>
inline T cfReflect(T src, T dst) {
    using namespace Arithmetic;
    
    
    return clamp<T>(cfGlow(dst,src));
}

template<class T>
inline T cfHeat(T src, T dst) {
    using namespace Arithmetic;
    
    if(src == unitValue<T>()) {
        return unitValue<T>();
    }

    if(dst == zeroValue<T>()) {
        return zeroValue<T>();
    }

    return inv(clamp<T>(div(mul(inv(src), inv(src)),dst)));
}

template<class T>
inline T cfFreeze(T src, T dst) {
    using namespace Arithmetic;
    
    return (cfHeat(dst,src)); 
}

template<class T>
inline T cfHelow(T src, T dst) {
    using namespace Arithmetic;
    // see http://www.pegtop.net/delphi/articles/blendmodes/quadratic.htm for formulas of Quadratic Blending Modes like Glow, Reflect, Freeze, and Heat
    
    if (cfHardMixPhotoshop(src,dst) == unitValue<T>()) {
        return cfHeat(src,dst);
    }
    
    if (src == zeroValue<T>()) {
        return zeroValue<T>();
    }

    return (cfGlow(src,dst));
}

template<class T>
inline T cfFrect(T src, T dst) {
    using namespace Arithmetic;

    if (cfHardMixPhotoshop(src,dst) == unitValue<T>()) {
        return cfFreeze(src,dst);
    }

    if (dst == zeroValue<T>()) {
        return zeroValue<T>();
    }

    return (cfReflect(src,dst));
}

template<class T>
inline T cfGleat(T src, T dst) {
    using namespace Arithmetic;
        // see http://www.pegtop.net/delphi/articles/blendmodes/quadratic.htm for formulas of Quadratic Blending Modes like Glow, Reflect, Freeze, and Heat
    
    if(dst == unitValue<T>()) {
        return unitValue<T>();
    }    
    
    if(cfHardMixPhotoshop(src,dst) == unitValue<T>()) {
        return cfGlow(src,dst);
    }
    
    return (cfHeat(src,dst));
}

template<class T>
inline T cfReeze(T src, T dst) {
    using namespace Arithmetic;
    
    return (cfGleat(dst,src)); 
}
template<class T>
inline T cfFhyrd(T src, T dst) {
    using namespace Arithmetic;
    
    return (cfAllanon(cfFrect(src,dst),cfHelow(src,dst))); 
}

template<class T>
inline T cfInterpolation(T src, T dst) {
    using namespace Arithmetic;

    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);
    
    if(dst == zeroValue<T>() && src == zeroValue<T>()) {
        return zeroValue<T>();
    } 

    return scale<T>(.5f-.25f*cos(pi*(fsrc))-.25f*cos(pi*(fdst)));
}

template<class T>
inline T cfInterpolationB(T src, T dst) {
    using namespace Arithmetic;

    return cfInterpolation(cfInterpolation(src,dst),cfInterpolation(src,dst));
}


template<class T>
inline T cfPenumbraB(T src, T dst) {
    using namespace Arithmetic;

    if (dst == unitValue<T>()) {
        return unitValue<T>();
    }    
    if (dst + src < unitValue<T>()) {
        return (cfColorDodge(dst,src)/2);
    }
    if (src == zeroValue<T>()) {
        return zeroValue<T>();
    }

    return inv(clamp<T>(div(inv(dst),src)/2));
}

template<class T>
inline T cfPenumbraD(T src, T dst) {
    using namespace Arithmetic;
    
    if (dst == unitValue<T>()) {
        return unitValue<T>();
    }
    
    return cfArcTangent(src,inv(dst));
}

template<class T>
inline T cfPenumbraC(T src, T dst) {
    using namespace Arithmetic;
    
    return cfPenumbraD(dst,src);
}

template<class T>
inline T cfPenumbraA(T src, T dst) {
    using namespace Arithmetic;

    return (cfPenumbraB(dst,src)); 
}

template<class T>
inline T cfSoftLightIFSIllusions(T src, T dst) {
    using namespace Arithmetic;
    
    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);
    
    return scale<T>(pow(fdst,pow(2.0,(mul(2.0,.5f-fsrc))))); 
}

template<class T>
inline T cfSoftLightPegtopDelphi(T src, T dst) {
    using namespace Arithmetic;

    return clamp<T>(cfAddition(mul(dst,cfScreen(src,dst)),mul(mul(src,dst),inv(dst)))); 
}

template<class T>
inline T cfNegation(T src, T dst) {
    using namespace Arithmetic;
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
        
    composite_type unit = unitValue<T>();
    composite_type a = unit - src - dst;
    composite_type s = abs(a);
    composite_type d = unit - s;
        
    return T(d);
}

template<class T>
inline T cfNor(T src, T dst) {
    using namespace Arithmetic;
        
    return and(src,dst);
}

template<class T>
inline T cfNand(T src, T dst) {
    using namespace Arithmetic;
        
    return or(src,dst);
}

template<class T>
inline T cfXor(T src, T dst) {
    using namespace Arithmetic;
    
    return xor(src,dst);
}

template<class T>
inline T cfXnor(T src, T dst) {
    using namespace Arithmetic;
    
    return cfXor(src,inv(dst));
}

template<class T>
inline T cfAnd(T src, T dst) {
    using namespace Arithmetic;
        
    return cfNor(inv(src),inv(dst));
}

template<class T>
inline T cfOr(T src, T dst) {
    using namespace Arithmetic;
        
    return cfNand(inv(src),inv(dst));
}

template<class T>
inline T cfConverse(T src, T dst) {
    using namespace Arithmetic;
        
    return cfOr(inv(src),dst);
}

template<class T>
inline T cfNotConverse(T src, T dst) {
    using namespace Arithmetic;
        
    return cfAnd(src,inv(dst));
}

template<class T>
inline T cfImplies(T src, T dst) {
    using namespace Arithmetic;
        
    return cfOr(src,inv(dst));
}

template<class T>
inline T cfNotImplies(T src, T dst) {
    using namespace Arithmetic;
        
    return cfAnd(inv(src),dst);
}

template<class T>
inline T cfPNormA(T src, T dst) {
    using namespace Arithmetic;
    //This is also known as P-Norm mode with factor of 2.3333 See IMBLEND image blending mode samples, and please see imblend.m file found on Additional Blending Mode thread at Phabricator. 1/2.3333 is .42875...
    
    return clamp<T>(pow(pow((float)dst, 2.3333333333333333) + pow((float)src, 2.3333333333333333), 0.428571428571434)); 
}

template<class T>
inline T cfPNormB(T src, T dst) {
    using namespace Arithmetic;
    //This is also known as P-Norm mode with factor of 2.3333 See IMBLEND image blending mode samples, and please see imblend.m file found on Additional Blending Mode thread at Phabricator. 1/2.3333 is .42875...
    
    return clamp<T>(pow(pow(dst,4)+pow(src,4),0.25)); 
}

template<class T>
inline T cfSuperLight(T src, T dst) {
    using namespace Arithmetic;
    //4.0 can be adjusted to taste. 4.0 is picked for being the best in terms of contrast and details. See imblend.m file.
        
    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);
    
    if (fsrc < .5) {
        return scale<T>(inv(pow(pow(inv(fdst),2.875)+pow(inv(2.0*fsrc),2.875),1.0/2.875)));
    }   
    
    return scale<T>(pow(pow(fdst,2.875)+pow(2.0*fsrc-1.0,2.875),1.0/2.875)); 
}

template<class T>
inline T cfTintIFSIllusions(T src, T dst) {
    using namespace Arithmetic;
    //Known as Light Blending mode found in IFS Illusions. Picked this name because it results into a very strong tint, and has better naming convention.
    
    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);
    
    return scale<T>(fsrc*inv(fdst)+sqrt(fdst)); 
}

template<class T>
inline T cfShadeIFSIllusions(T src, T dst) {
    using namespace Arithmetic;
    //Known as Shadow Blending mode found in IFS Illusions. Picked this name because it is the opposite of Tint (IFS Illusion Blending mode).
    
    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);
    
    return scale<T>(inv((inv(fdst)*fsrc)+sqrt(inv(fsrc)))); 
}

template<class T>
inline T cfFogLightenIFSIllusions(T src, T dst) {
    using namespace Arithmetic;
    //Known as Bright Blending mode found in IFS Illusions. Picked this name because the shading reminds me of fog when overlaying with a gradientt.
    
    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);
    
    if (fsrc < .5) {
        return scale<T>(inv(inv(fsrc)*fsrc)-inv(fdst)*inv(fsrc));
    }  
    
    return scale<T>(fsrc-inv(fdst)*inv(fsrc)+pow(inv(fsrc),2)); 
}

template<class T>
inline T cfFogDarkenIFSIllusions(T src, T dst) {
    using namespace Arithmetic;
    //Known as Dark Blending mode found in IFS Illusions. Picked this name because the shading reminds me of fog when overlaying with a gradient.
    
    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);
    
    if (fsrc < .5) {
        return scale<T>(inv(fsrc)*fsrc+fsrc*fdst);
    }  
    
    return scale<T>(fsrc*fdst+fsrc-pow(fsrc,2)); 
}

template<class T>
inline T cfModulo(T src, T dst) {
    using namespace Arithmetic;
    
    return mod(dst,src); 
}

template<class T>
inline T cfModuloShift(T src, T dst) {
    using namespace Arithmetic;
    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);
    
    if (fsrc == 1.0 && fdst == 0.0) {
    return scale<T>(0.0);
    }  

    
    return scale<T>(mod((fdst+fsrc),1.0000000000)); 
}

template<class T>
inline T cfModuloShiftContinuous(T src, T dst) {
    using namespace Arithmetic;
    //This blending mode do not behave like difference/equilavent with destination layer inverted if you use group layer on addition while the content of group layer contains several addition-mode layers, it works as expected on float images. So, no need to change this.
    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);
    
    if (fsrc == 1.0 && fdst == 0.0) {
    return scale<T>(0.0);
    }  
    
    return scale<T>((int(ceil(fdst+fsrc)) % 2 != 0) || (fdst == zeroValue<T>()) ? inv(cfModuloShift(fsrc,fdst)) : cfModuloShift(fsrc,fdst)); 
}

template<class T>
inline T cfDivisiveModulo(T src, T dst) {
    using namespace Arithmetic;
    //I have to use 1.00000 as unitValue failed to work for those area.
    
    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);
    
        if (fsrc == zeroValue<T>()) {
        return scale<T>(mod(((1.0000000000/epsilon<T>()) * fdst),1.0000000000));
    }  
    
    return scale<T>(mod(((1.0000000000/fsrc) * fdst),1.0000000000)); 
}

template<class T>
inline T cfDivisiveModuloContinuous(T src, T dst) {
    using namespace Arithmetic;
    
    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);
    
    if (fdst == zeroValue<T>()) {
    return zeroValue<T>();
    }  
    
    if (fsrc == zeroValue<T>()) {
    return cfDivisiveModulo(fsrc,fdst);
    }  

    
    return scale<T>( int(ceil(fdst/fsrc)) % 2 != 0 ? cfDivisiveModulo(fsrc,fdst) : inv(cfDivisiveModulo(fsrc,fdst))); 
}

template<class T>
inline T cfModuloContinuous(T src, T dst) {
    using namespace Arithmetic;
    
    return cfMultiply(cfDivisiveModuloContinuous(src,dst),src); 
}

template<class T>
inline T cfEasyDodge(T src, T dst) {
    using namespace Arithmetic;
    // The 13 divided by 15 can be adjusted to taste. See imgblend.m
    
    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);
    
    if (fsrc == 1.0) {
        return scale<T>(1.0);}

    
    return scale<T>(pow(fdst,mul(inv(fsrc != 1.0 ? fsrc : .999999999999),1.039999999))); 
}

template<class T>
inline T cfEasyBurn(T src, T dst) {
    using namespace Arithmetic;
    // The 13 divided by 15 can be adjusted to taste. See imgblend.m
    
    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);

    
    return scale<T>(inv(pow(inv(fsrc != 1.0 ? fsrc : .999999999999),mul(fdst,1.039999999)))); 
}

template<class T>
inline T cfFlatLight(T src, T dst) {
    using namespace Arithmetic;
    
    if (src == zeroValue<T>()) {
    return zeroValue<T>();
    }  
    
    return clamp<T>(cfHardMixPhotoshop(inv(src),dst)==unitValue<T>() ? cfPenumbraB(src,dst) : cfPenumbraA(src,dst)); 
}
#endif // KOCOMPOSITEOP_FUNCTIONS_H_
