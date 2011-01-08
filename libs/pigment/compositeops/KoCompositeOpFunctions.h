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

#ifndef KOCOMPOSITEOPFUNCTIONS_H_
#define KOCOMPOSITEOPFUNCTIONS_H_

#include <KoColorSpaceMaths.h>
#include <KoColorSpaceConstants.h>
#include <KoCompositeOp.h>

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

/* ------------------------ Auxiliary Functions --------------------------- /
 * definitions of auxiliary functions needed by the blending functions
 * or the KoCompositeOp* classes to calculate the pixel colors
 */

template<class T>
inline T unionShapeOpacy(T a, T b) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    return T(composite_type(a) + b - mul(a,b));
}

template<class T, T blendFunc(T,T)>
inline T blend(T src, T srcAlpha, T dst, T dstAlpha) {
    return mul(inv(srcAlpha), dstAlpha, dst) + mul(inv(dstAlpha), srcAlpha, src) + mul(dstAlpha, srcAlpha, blendFunc(src, dst));
}

/* ---------------- Blending/Compositing functions ------------------------ /
 * definitions of standard blending/compositing functions compatible
 * to the ISO 32000-1 specification (for PDF filed) which also defines
 * the compositing functions who are also used in Adobe Photoshop (c)
 */

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
            return KoColorSpaceMathsTraits<T>::zeroValue; //TODO: maybe better to return unitValue, must be verified
            
        // min(1,max(0,1-(1-dst) / (2*src)))
        composite_type src2 = composite_type(src) + src;
        composite_type dsti = inv(dst);
        return clamp<T>(KoColorSpaceMathsTraits<T>::unitValue - (dsti * KoColorSpaceMathsTraits<T>::unitValue / src2));
    }
    
    if(src == KoColorSpaceMathsTraits<T>::unitValue)
        return KoColorSpaceMathsTraits<T>::unitValue; //TODO: maybe better to return zeroValue, must be verified
    
    // min(1,max(0, dst / (2*(1-src)))
    composite_type srci2 = inv(src);
    srci2 += srci2;
    return clamp<T>(composite_type(dst) * KoColorSpaceMathsTraits<T>::unitValue / srci2);
}

template<class T>
inline T cfPinLight(T src, T dst) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    // TODO: verify that the formular is correct (the first max would be useless here)
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

/**
 * A template base class that can be used for most composite modes/ops
 *
 * @param _compositeOp this template parameter is a class that must be
 *        derived fom KoCompositeOpBase and must define the static member function
 *        template<bool alphaLocked, bool allChannelFlags>
 *        inline static channels_type composeColorChannels(
 *            const channels_type* src,
 *            channels_type srcAlpha,
 *            channels_type* dst,
 *            channels_type dstAlpha,
 *            channels_type opacity,
 *            const QBitArray& channelFlags
 *        )
 *
 *        where channels_type is _CSTraits::channels_type
 */
template<class _CSTraits, class _compositeOp>
class KoCompositeOpBase : public KoCompositeOp
{
    typedef typename _CSTraits::channels_type channels_type;
    static const qint32 channels_nb = _CSTraits::channels_nb;
    static const qint32 alpha_pos   = _CSTraits::alpha_pos;
    
public:
    
    KoCompositeOpBase(const KoColorSpace* cs, const QString& id, const QString& description, const QString& category, bool userVisible)
        : KoCompositeOp(cs, id, description, category, userVisible) { }
    
private:
    template<bool alphaLocked, bool allChannelFlags>
    void genericComposite(quint8*       dstRowStart , qint32 dstRowStride ,
                          const quint8* srcRowStart , qint32 srcRowStride ,
                          const quint8* maskRowStart, qint32 maskRowStride,
                          qint32 rows, qint32 cols, quint8 U8_opacity, const QBitArray& channelFlags) const {
        
        qint32        srcInc    = (srcRowStride == 0) ? 0 : channels_nb;
        bool          useMask   = maskRowStart != 0;
        channels_type unitValue = KoColorSpaceMathsTraits<channels_type>::unitValue;
        channels_type opacity   = KoColorSpaceMaths<quint8,channels_type>::scaleToA(U8_opacity);
        
        for(; rows>0; --rows) {
            const channels_type* src  = reinterpret_cast<const channels_type*>(srcRowStart);
            channels_type*       dst  = reinterpret_cast<channels_type*>(dstRowStart);
            const quint8*        mask = maskRowStart;
            
            for(qint32 c=cols; c>0; --c) {
                channels_type srcAlpha = (alpha_pos == -1) ? unitValue : src[alpha_pos];
                channels_type dstAlpha = (alpha_pos == -1) ? unitValue : dst[alpha_pos];
                channels_type blend    = useMask ? mul(opacity, scale<channels_type>(*mask)) : opacity;
                
                channels_type newDstAlpha = _compositeOp::template composeColorChannels<alphaLocked,allChannelFlags>(
                    src, srcAlpha, dst, dstAlpha, blend, channelFlags
                );
                
                if(alpha_pos != -1)
                    dst[alpha_pos] = alphaLocked ? dstAlpha : newDstAlpha;
                
                src += srcInc;
                dst += channels_nb;
                ++mask;
            }
            
            srcRowStart  += srcRowStride;
            dstRowStart  += dstRowStride;
            maskRowStart += maskRowStride;
        }
    }

public:
    using KoCompositeOp::composite;
    
    virtual void composite(quint8*       dstRowStart , qint32 dstRowStride ,
                           const quint8* srcRowStart , qint32 srcRowStride ,
                           const quint8* maskRowStart, qint32 maskRowStride,
                           qint32 rows, qint32 cols, quint8 U8_opacity, const QBitArray& channelFlags) const {
        
        const QBitArray& flags           = channelFlags.isEmpty() ? QBitArray(channels_nb,true) : channelFlags;
        bool             allChannelFlags = channelFlags.isEmpty();
        bool             alphaLocked     = (alpha_pos != -1) && !flags.testBit(alpha_pos);
        
        if(alphaLocked) {
            if(allChannelFlags)
                genericComposite<true,true>(dstRowStart, dstRowStride, srcRowStart, srcRowStride, maskRowStart, maskRowStride, rows, cols, U8_opacity, flags);
            else
                genericComposite<true,false>(dstRowStart, dstRowStride, srcRowStart, srcRowStride, maskRowStart, maskRowStride, rows, cols, U8_opacity, flags);
        }
        else {
            if(allChannelFlags)
                genericComposite<false,true>(dstRowStart, dstRowStride, srcRowStart, srcRowStride, maskRowStart, maskRowStride, rows, cols, U8_opacity, flags);
            else
                genericComposite<false,false>(dstRowStart, dstRowStride, srcRowStart, srcRowStride, maskRowStart, maskRowStride, rows, cols, U8_opacity, flags);
        }
    }
};

#endif // KOCOMPOSITEOPFUNCTIONS_H_
