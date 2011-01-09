/*
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

#ifndef KOCOMPOSITEOP_FUNCTIONS_H_
#define KOCOMPOSITEOP_FUNCTIONS_H_

#include <KoColorSpaceMaths.h>

/* --------------------- Arithmetic functions ----------------------------- /
 * definitions of standard arithmetic functions. all computations are meant
 * to be performed on normalized values (in the range of 0.0 - 1.0)
 * if non floating point types are used, fixed point arithmetic is used
 */

// template<class T>
// inline T mul(T a, T b) { T(KoColorSpaceMaths<T>::multiply(a, b)); }

// template<class T>
// inline T mul(T a, T b, T c) { T(KoColorSpaceMaths<T>::multiply(a, b, c)); }

template<class T>
inline T mul(T a, T b) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    return T(composite_type(a) * b / KoColorSpaceMathsTraits<T>::unitValue);
}

template<class T>
inline T mul(T a, T b, T c) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    return T((composite_type(a) * b * c) / (composite_type(KoColorSpaceMathsTraits<T>::unitValue) * KoColorSpaceMathsTraits<T>::unitValue));
}

template<class T>
inline T inv(T a) { return KoColorSpaceMathsTraits<T>::unitValue - a; }

template<class T>
inline T lerp(T a, T b, T alpha) { return KoColorSpaceMaths<T>::blend(b, a, alpha); }

template<class TRet, class T>
inline TRet scale(T a) { return KoColorSpaceMaths<T,TRet>::scaleToA(a); }

// template<class TRet, class T>
// inline TRet scale(T a) {
//     typedef typename KoColorSpaceMathsTraits<TRet>::compositetype composite_type;
//     return TRet(composite_type(a) * KoColorSpaceMathsTraits<TRet>::unitValue / KoColorSpaceMathsTraits<T>::unitValue);
// }

template<class T>
inline typename KoColorSpaceMathsTraits<T>::compositetype
div(T a, T b) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    return composite_type(a) * KoColorSpaceMathsTraits<T>::unitValue / composite_type(b);
}

template<class T>
inline T clamp(typename KoColorSpaceMathsTraits<T>::compositetype a) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    return qBound<composite_type>(KoColorSpaceMathsTraits<T>::zeroValue, a, KoColorSpaceMathsTraits<T>::unitValue);
}

template<class T>
inline T min(T a, T b, T c) {
    b = (a < b) ? a : b;
    return (b < c) ? b : c;
}

template<class T>
inline T max(T a, T b, T c) {
    b = (a > b) ? a : b;
    return (b > c) ? b : c;
}

/* ------------------------ Auxiliary Functions --------------------------- /
 * definitions of auxiliary functions needed by the blending functions
 * or the KoCompositeOp* classes to calculate the pixel colors
 */

template<class T>
inline T unionShapeOpacy(T a, T b) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    return T(composite_type(a) + b - mul(a,b));
}

template<class T>
inline T blend(T src, T srcAlpha, T dst, T dstAlpha, T cfValue) {
    return mul(inv(srcAlpha), dstAlpha, dst) + mul(inv(dstAlpha), srcAlpha, src) + mul(dstAlpha, srcAlpha, cfValue);
}

template<class TReal>
inline TReal getLuminosity(TReal r, TReal g, TReal b) {
    return TReal(0.3)*r + TReal(0.59)*g + TReal(0.11)*b;
}

template<class TReal>
inline void setLuminosity(TReal& r, TReal& g, TReal& b, TReal lum) {
    
    TReal d = lum - getLuminosity(r, g, b);
    
    r += d;
    g += d;
    b += d;
    
    TReal l = getLuminosity(r, g, b);
    TReal n = min(r, g, b);
    TReal x = max(r, g, b);
    
    if(n < TReal(0.0)) {
        r = l + ((r-l) * l) / (l-n);
        g = l + ((g-l) * l) / (l-n);
        b = l + ((b-l) * l) / (l-n);
    }
    
    if(x > TReal(1.0)) {
        r = l + ((r-l) * (TReal(1.0)-l)) / (x-l);
        g = l + ((g-l) * (TReal(1.0)-l)) / (x-l);
        b = l + ((b-l) * (TReal(1.0)-l)) / (x-l);
    }
}

template<class TReal>
inline TReal getSaturation(TReal r, TReal g, TReal b) {
    return max(r,g,b) - min(r,g,b);
}

template<class TReal>
inline void setSaturation(TReal& r, TReal& g, TReal& b, TReal sat) {
    TReal& min = r;
    TReal& mid = g;
    TReal& max = b;
    
    if(mid < min) {
        TReal& tmp = min;
        min = mid;
        mid = tmp;
    }
    
    if(max < mid) {
        TReal& tmp = mid;
        mid = max;
        max = tmp;
    }
    
    if(mid < min) {
        TReal& tmp = min;
        min = mid;
        mid = tmp;
    }
    
    if(max > min) {
        mid = ((mid-min) * sat) / (max-min);
        max = sat;
    } else {
        mid = TReal(0.0);
        max = TReal(0.0);
    }
    
    min = TReal(0.0);
}

/* ---------------- Blending/Compositing functions ------------------------ /
 * definitions of standard blending/compositing functions compatible
 * to the ISO 32000-1 specification (for PDF files) which also defines
 * the compositing functions who are also used in Adobe Photoshop (c)
 */

template<class TReal>
inline void cfColor(TReal sr, TReal sg, TReal sb, TReal& dr, TReal& dg, TReal& db) {
    TReal lum = getLuminosity(dr, dg, db);
    dr = sr;
    dg = sg;
    db = sb;
    setLuminosity(dr, dg, db, lum);
}

template<class TReal>
inline void cfLuminosity(TReal sr, TReal sg, TReal sb, TReal& dr, TReal& dg, TReal& db) {
    setLuminosity(dr, dg, db, getLuminosity(sr, sg, sb));
}

template<class TReal>
inline void cfSaturation(TReal sr, TReal sg, TReal sb, TReal& dr, TReal& dg, TReal& db) {
    TReal sat = getSaturation(sr, sg, sb);
    TReal lum = getLuminosity(dr, dg, db);
    setSaturation(dr, dg, db, sat);
    setLuminosity(dr, dg, db, lum);
}

template<class TReal>
inline void cfHue(TReal sr, TReal sg, TReal sb, TReal& dr, TReal& dg, TReal& db) {
    TReal sat = getSaturation(dr, dg, db);
    TReal lum = getLuminosity(dr, dg, db);
    dr = sr;
    dg = sg;
    db = sb;
    setSaturation(dr, dg, db, sat);
    setLuminosity(dr, dg, db, lum);
}

template<class T>
inline T cfColorBurn(T src, T dst) {
    if(src != KoColorSpaceMathsTraits<T>::zeroValue)
        return inv(clamp<T>(div(inv(dst), src)));
    return KoColorSpaceMathsTraits<T>::zeroValue;
}

template<class T>
inline T cfLinearBurn(T src, T dst) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    return clamp<T>(composite_type(src) + dst - KoColorSpaceMathsTraits<T>::unitValue);
}

template<class T>
inline T cfColorDodge(T src, T dst) {
    if(src != KoColorSpaceMathsTraits<T>::unitValue)
        return clamp<T>(div(dst, inv(src)));
    return KoColorSpaceMathsTraits<T>::unitValue;
}

template<class T>
inline T cfAddition(T src, T dst) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    return clamp<T>(composite_type(src) + dst);
}

template<class T>
inline T cfSubtract(T src, T dst) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    return clamp<T>(composite_type(dst) - src);
}

template<class T>
inline T cfExclusion(T src, T dst) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    composite_type x = mul(src, dst);
    return clamp<T>(composite_type(dst) + src - (x + x));
}

template<class T>
inline T cfDivide(T src, T dst) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    if(src == KoColorSpaceMathsTraits<T>::zeroValue)
        return dst;
    return clamp<T>(div(dst, src));
}

template<class T>
inline T cfHardLight(T src, T dst) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    composite_type src2 = composite_type(src) + src;
    
    if(src > KoColorSpaceMathsTraits<T>::halfValue) {
        // screen(src*2.0 - 1.0, dst)
        src2 -= KoColorSpaceMathsTraits<T>::unitValue;
        return T((src2+dst) - (src2*dst / KoColorSpaceMathsTraits<T>::unitValue));
    }
    
    // multiply(src*2.0, dst)
    return clamp<T>(src2*dst / KoColorSpaceMathsTraits<T>::unitValue);
}

template<class T>
inline T cfSoftLight(T src, T dst) {
    qreal fsrc = scale<qreal>(src);
    qreal fdst = scale<qreal>(dst);
    
    if(fsrc > 0.5f) {
        qreal D = (fdst > 0.25f) ? sqrt(fdst) : ((16.0f*fdst - 12.0)*fdst + 4.0f)*fdst;
        return scale<T>(fdst + (2.0f*fsrc - 1.0f) * (D - fdst));
    }
    
    return scale<T>(fdst - (1.0f - 2.0f*fsrc) * fdst * (1.0f - fdst));
}

template<class T>
inline T cfVividLight(T src, T dst) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    
    if(src < KoColorSpaceMathsTraits<T>::halfValue) {
    
        if(src == KoColorSpaceMathsTraits<T>::zeroValue)
            return (dst == KoColorSpaceMathsTraits<T>::unitValue) ?
                KoColorSpaceMathsTraits<T>::unitValue : KoColorSpaceMathsTraits<T>::zeroValue;
            
        // min(1,max(0,1-(1-dst) / (2*src)))
        composite_type src2 = composite_type(src) + src;
        composite_type dsti = inv(dst);
        return clamp<T>(KoColorSpaceMathsTraits<T>::unitValue - (dsti * KoColorSpaceMathsTraits<T>::unitValue / src2));
    }
    
    if(src == KoColorSpaceMathsTraits<T>::unitValue)
        return (dst == KoColorSpaceMathsTraits<T>::zeroValue) ?
            KoColorSpaceMathsTraits<T>::zeroValue : KoColorSpaceMathsTraits<T>::unitValue;
    
    // min(1,max(0, dst / (2*(1-src)))
    composite_type srci2 = inv(src);
    srci2 += srci2;
    return clamp<T>(composite_type(dst) * KoColorSpaceMathsTraits<T>::unitValue / srci2);
}

template<class T>
inline T cfPinLight(T src, T dst) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    // TODO: verify that the formula is correct (the first max would be useless here)
    // max(0, max(2*src-1, min(dst, 2*src)))
    composite_type src2 = composite_type(src) + src;
    composite_type a    = qMin<composite_type>(dst, src2);
    composite_type b    = qMax<composite_type>(src2-KoColorSpaceMathsTraits<T>::unitValue, a);
    return T(b);
}

template<class T>
inline T cfArcTangent(T src, T dst) {
    const static qreal pi = 3.14159265358979323846;
    
    if(dst == KoColorSpaceMathsTraits<T>::zeroValue)
        return (src == KoColorSpaceMathsTraits<T>::zeroValue) ?
            KoColorSpaceMathsTraits<T>::zeroValue : KoColorSpaceMathsTraits<T>::unitValue;
    
    return scale<T>(2.0 * atan(scale<qreal>(src) / scale<qreal>(dst)) / pi);
}

template<class T>
inline T cfAllanon(T src, T dst) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    // (dst + src) / 2   [or (dst + src) * 0.5]
    return T((composite_type(src) + dst) * KoColorSpaceMathsTraits<T>::halfValue / KoColorSpaceMathsTraits<T>::unitValue);
}

template<class T>
inline T cfLinearLight(T src, T dst) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    // min(1,max(0,(dst + 2*src)-1))
    return clamp<T>((composite_type(src) + src + dst) - KoColorSpaceMathsTraits<T>::unitValue);
}

template<class T>
inline T cfParallel(T src, T dst) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    
    // min(max(2 / (1/dst + 1/src), 0), 1)
    composite_type unit = KoColorSpaceMathsTraits<T>::unitValue;
    composite_type s    = (src != KoColorSpaceMathsTraits<T>::zeroValue) ? div<T>(unit, src) : unit;
    composite_type d    = (dst != KoColorSpaceMathsTraits<T>::zeroValue) ? div<T>(unit, dst) : unit;
    
    return clamp<T>((unit+unit) * KoColorSpaceMathsTraits<T>::unitValue / (d+s));
}

template<class T>
inline T cfEquivalence(T src, T dst) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    // 1 - abs(dst - src)
    composite_type x = composite_type(dst) - src;
    return (x < KoColorSpaceMathsTraits<T>::zeroValue) ? T(-x) : T(x);
}

template<class T>
inline T cfGammaDark(T src, T dst) {
    if(src == KoColorSpaceMathsTraits<T>::zeroValue)
        return KoColorSpaceMathsTraits<T>::zeroValue;
    
    // power(dst, 1/src)
    return scale<T>(pow(scale<qreal>(dst), 1.0/scale<qreal>(src)));
}

template<class T>
inline T cfGammaLight(T src, T dst) { return scale<T>(pow(scale<qreal>(dst), scale<qreal>(src))); }

template<class T>
inline T cfGeometricMean(T src, T dst) { return scale<T>(sqrt(scale<qreal>(dst) * scale<qreal>(src))); }

template<class T>
inline T cfOver(T src, T dst) { Q_UNUSED(dst); return src; }

template<class T>
inline T cfOverlay(T src, T dst) { return cfHardLight(dst, src); }

template<class T>
inline T cfMultiply(T src, T dst) { return mul(src, dst); }

template<class T>
inline T cfDifference(T src, T dst) { return qMax(src,dst) - qMin(src,dst); }

template<class T>
inline T cfScreen(T src, T dst) { return unionShapeOpacy(src, dst); }

template<class T>
inline T cfDarkenOnly(T src, T dst) { return qMin(src, dst); }

template<class T>
inline T cfLightenOnly(T src, T dst) { return qMax(src, dst); }

#endif // KOCOMPOSITEOP_FUNCTIONS_H_
