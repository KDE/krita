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

template<class T>
inline T mul(T a, T b) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    return T(composite_type(a) * composite_type(b) / KoColorSpaceMathsTraits<T>::unitValue);
}

template<class T>
inline T mul(T a, T b, T c) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    return T((composite_type(a) * b * c) / (composite_type(KoColorSpaceMathsTraits<T>::unitValue) * KoColorSpaceMathsTraits<T>::unitValue));
}

template<class T>
inline typename KoColorSpaceMathsTraits<T>::compositetype
div(T a, T b) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    return composite_type(a) * KoColorSpaceMathsTraits<T>::unitValue / composite_type(b);
}

template<class T>
inline T inv(T a) {
    return KoColorSpaceMathsTraits<T>::unitValue - a;
}

template<class T>
inline T clamp(typename KoColorSpaceMathsTraits<T>::compositetype a) {
    return (a > KoColorSpaceMathsTraits<T>::unitValue) ? KoColorSpaceMathsTraits<T>::unitValue : T(a);
}

template<class T>
inline T lerp(T a, T b, T alpha) {
    return KoColorSpaceMaths<T>::blend(b, a, alpha);
}

template<class TRet, class T>
inline TRet scale(T a) {
    typedef typename KoColorSpaceMathsTraits<TRet>::compositetype composite_type;
    return TRet(composite_type(a) * KoColorSpaceMathsTraits<TRet>::unitValue / KoColorSpaceMathsTraits<T>::unitValue);
}

/* ---------------- Blending/Compositing functions ------------------------ /
 * definitions of standard blending/compositing functions compatible
 * to the ISO 32000-1 specification (for PDF filed) which also defines
 * the compositing functions who are also used in Adobe Photoshop (c)
 */

template<class T>
inline T unionShapeOpacy(T a, T b) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    return T(composite_type(a) + b - mul(a,b));
}

template<class T, class TFunc>
inline T blend(T src, T srcAlpha, T dst, T dstAlpha, TFunc blendFunc) {
    return mul(inv(srcAlpha), dstAlpha, dst) + mul(inv(dstAlpha), srcAlpha, src) + mul(dstAlpha, srcAlpha, blendFunc(src, dst));
}

template<class T>
inline T cfColorBurn(T src, T dst) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    
    if(src != KoColorSpaceMathsTraits<T>::zeroValue)
        return inv(clamp<T>(div(inv(dst), src)));
    
    return KoColorSpaceMathsTraits<T>::zeroValue;
}

template<class T>
inline T cfColorDodge(T src, T dst) {
    typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
    
    if(src != KoColorSpaceMathsTraits<T>::unitValue)
        return clamp<T>(div(dst, inv(src)));
    
    return KoColorSpaceMathsTraits<T>::unitValue;
}

/**
 * A template base class that can be used for most composite modes/ops
 *
 * @param _compositeOp this template parameter is a class that must be
 *        derived fom KoCompositeOpBase and must define the static member function
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
    
    KoCompositeOpBase(const KoColorSpace* cs, const QString& id, const QString& description, const QString& category)
        : KoCompositeOp(cs, id, description, category) { }
    
private:
    template<bool alphaLocked>
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
                channels_type srcAlpha    = (alpha_pos == -1) ? unitValue : src[alpha_pos];
                channels_type dstAlpha    = (alpha_pos == -1) ? unitValue : dst[alpha_pos];
                channels_type blend       = useMask ? mul(opacity, scale<channels_type>(*mask)) : opacity;
                channels_type newDstAlpha = _compositeOp::composeColorChannels(src, srcAlpha, dst, dstAlpha, blend, channelFlags);
                
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
        
        const QBitArray& flags       = channelFlags.isEmpty() ? QBitArray(channels_nb,true) : channelFlags;
        bool             alphaLocked = (alpha_pos != -1) && !flags.testBit(alpha_pos);
        
        if(alphaLocked)
            genericComposite<true>(dstRowStart, dstRowStride, srcRowStart, srcRowStride, maskRowStart, maskRowStride, rows, cols, U8_opacity, flags);
        else
            genericComposite<false>(dstRowStart, dstRowStride, srcRowStart, srcRowStride, maskRowStart, maskRowStride, rows, cols, U8_opacity, flags);
    }
};

#endif // KOCOMPOSITEOPFUNCTIONS_H_
