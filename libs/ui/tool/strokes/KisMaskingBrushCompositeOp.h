/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISMASKINGBRUSHCOMPOSITEOP_H
#define KISMASKINGBRUSHCOMPOSITEOP_H

#include <type_traits>

#include <KoColorSpaceTraits.h>
#include <KoGrayColorSpaceTraits.h>
#include <KoColorSpaceMaths.h>
#include <KoCompositeOpFunctions.h>
#include <kritaui_export.h>

#include "KisMaskingBrushCompositeOpBase.h"

enum KisMaskingBrushCompositeFuncTypes
{
    KIS_MASKING_BRUSH_COMPOSITE_MULT,
    KIS_MASKING_BRUSH_COMPOSITE_DARKEN,
    KIS_MASKING_BRUSH_COMPOSITE_OVERLAY,
    KIS_MASKING_BRUSH_COMPOSITE_DODGE,
    KIS_MASKING_BRUSH_COMPOSITE_BURN,
    KIS_MASKING_BRUSH_COMPOSITE_LINEAR_BURN,
    KIS_MASKING_BRUSH_COMPOSITE_LINEAR_DODGE,
    KIS_MASKING_BRUSH_COMPOSITE_HARD_MIX_PHOTOSHOP,
    KIS_MASKING_BRUSH_COMPOSITE_HARD_MIX_SOFTER_PHOTOSHOP,
    KIS_MASKING_BRUSH_COMPOSITE_SUBTRACT,
    KIS_MASKING_BRUSH_COMPOSITE_HEIGHT,
    KIS_MASKING_BRUSH_COMPOSITE_LINEAR_HEIGHT,
    KIS_MASKING_BRUSH_COMPOSITE_HEIGHT_PHOTOSHOP,
    KIS_MASKING_BRUSH_COMPOSITE_LINEAR_HEIGHT_PHOTOSHOP
};

namespace KisMaskingBrushCompositeDetail
{

template <typename channels_type>
struct StrengthCompositeFunctionBase
{
    const channels_type strength;
    StrengthCompositeFunctionBase(qreal strength)
        : strength(KoColorSpaceMaths<qreal, channels_type>::scaleToA(strength))
    {}
};

template <typename channels_type, int composite_function, bool use_strength>
struct CompositeFunction;

template <typename channels_type>
struct CompositeFunction<channels_type, KIS_MASKING_BRUSH_COMPOSITE_MULT, false>
{
    channels_type apply(channels_type src, channels_type dst)
    {
        return cfMultiply(src, dst);
    }
};

template <typename channels_type>
struct CompositeFunction<channels_type, KIS_MASKING_BRUSH_COMPOSITE_MULT, true> : public StrengthCompositeFunctionBase<channels_type>
{
    CompositeFunction(qreal strength) : StrengthCompositeFunctionBase<channels_type>(strength) {}
    
    channels_type apply(channels_type src, channels_type dst)
    {
        return Arithmetic::mul(src, dst, StrengthCompositeFunctionBase<channels_type>::strength);
    }
};

template <typename channels_type>
struct CompositeFunction<channels_type, KIS_MASKING_BRUSH_COMPOSITE_DARKEN, false>
{
    channels_type apply(channels_type src, channels_type dst)
    {
        return cfDarkenOnly(src, dst);
    }
};

template <typename channels_type>
struct CompositeFunction<channels_type, KIS_MASKING_BRUSH_COMPOSITE_DARKEN, true> : public StrengthCompositeFunctionBase<channels_type>
{
    CompositeFunction(qreal strength) : StrengthCompositeFunctionBase<channels_type>(strength) {}
    
    channels_type apply(channels_type src, channels_type dst)
    {
        return cfDarkenOnly(src, Arithmetic::mul(dst, StrengthCompositeFunctionBase<channels_type>::strength));
    }
};

template <typename channels_type>
struct CompositeFunction<channels_type, KIS_MASKING_BRUSH_COMPOSITE_OVERLAY, false>
{
    channels_type apply(channels_type src, channels_type dst)
    {
        return cfOverlay(src, dst);
    }
};

template <typename channels_type>
struct CompositeFunction<channels_type, KIS_MASKING_BRUSH_COMPOSITE_OVERLAY, true> : public StrengthCompositeFunctionBase<channels_type>
{
    CompositeFunction(qreal strength) : StrengthCompositeFunctionBase<channels_type>(strength) {}
    
    channels_type apply(channels_type src, channels_type dst)
    {
        return cfOverlay(src, Arithmetic::mul(dst, StrengthCompositeFunctionBase<channels_type>::strength));
    }
};

template <typename channels_type>
struct CompositeFunction<channels_type, KIS_MASKING_BRUSH_COMPOSITE_DODGE, false>
{
    channels_type apply(channels_type src, channels_type dst)
    {
        return cfColorDodge(src, dst);
    }
};

template <typename channels_type>
struct CompositeFunction<channels_type, KIS_MASKING_BRUSH_COMPOSITE_DODGE, true> : public StrengthCompositeFunctionBase<channels_type>
{
    CompositeFunction(qreal strength) : StrengthCompositeFunctionBase<channels_type>(strength) {}
    
    channels_type apply(channels_type src, channels_type dst)
    {
        return cfColorDodge(src, Arithmetic::mul(dst, StrengthCompositeFunctionBase<channels_type>::strength));
    }
};

template <typename channels_type>
struct CompositeFunction<channels_type, KIS_MASKING_BRUSH_COMPOSITE_BURN, false>
{
    channels_type apply(channels_type src, channels_type dst)
    {
        return cfColorBurn(src, dst);
    }
};

template <typename channels_type>
struct CompositeFunction<channels_type, KIS_MASKING_BRUSH_COMPOSITE_BURN, true> : public StrengthCompositeFunctionBase<channels_type>
{
    CompositeFunction(qreal strength) : StrengthCompositeFunctionBase<channels_type>(strength) {}
    
    channels_type apply(channels_type src, channels_type dst)
    {
        return cfColorBurn(src, Arithmetic::mul(dst, StrengthCompositeFunctionBase<channels_type>::strength));
    }
};

/**
 * A special Linear Dodge variant for alpha channel.
 *
 * The meaning of alpha channel is a bit different from the one in color. If
 * alpha channel of the destination is totally null, we should not try
 * to resurrect its contents from ashes :)
 */
template <typename channels_type>
struct CompositeFunction<channels_type, KIS_MASKING_BRUSH_COMPOSITE_LINEAR_DODGE, false>
{
    channels_type apply(channels_type src, channels_type dst)
    {
        using composite_type = typename KoColorSpaceMathsTraits<channels_type>::compositetype;
        using namespace Arithmetic;
        if (dst == zeroValue<channels_type>()) {
            return zeroValue<channels_type>();
        }
        return qBound(composite_type(KoColorSpaceMathsTraits<channels_type>::zeroValue),
                      composite_type(src) + dst,
                      composite_type(KoColorSpaceMathsTraits<channels_type>::unitValue));
    }
};

template <typename channels_type>
struct CompositeFunction<channels_type, KIS_MASKING_BRUSH_COMPOSITE_LINEAR_DODGE, true> : public StrengthCompositeFunctionBase<channels_type>
{
    CompositeFunction(qreal strength) : StrengthCompositeFunctionBase<channels_type>(strength) {}
    
    channels_type apply(channels_type src, channels_type dst)
    {
        using composite_type = typename KoColorSpaceMathsTraits<channels_type>::compositetype;
        using namespace Arithmetic;
        if (dst == zeroValue<channels_type>()) {
            return zeroValue<channels_type>();
        }
        return qBound(composite_type(KoColorSpaceMathsTraits<channels_type>::zeroValue),
                      composite_type(src) + mul(dst, StrengthCompositeFunctionBase<channels_type>::strength),
                      composite_type(KoColorSpaceMathsTraits<channels_type>::unitValue));
    }
};

/**
 * A special Linear Burn variant for alpha channel
 *
 * The meaning of alpha channel is a bit different from the one in color. We should
 * clamp the values around [zero, max] only to avoid the brush to **erase** the content
 * of the layer below
 */
template <typename channels_type>
struct CompositeFunction<channels_type, KIS_MASKING_BRUSH_COMPOSITE_LINEAR_BURN, false>
{
    channels_type apply(channels_type src, channels_type dst)
    {
        using namespace Arithmetic;
        using composite_type = typename KoColorSpaceMathsTraits<channels_type>::compositetype;
        return qBound(composite_type(KoColorSpaceMathsTraits<channels_type>::zeroValue),
                      composite_type(src) + dst - unitValue<channels_type>(),
                      composite_type(KoColorSpaceMathsTraits<channels_type>::unitValue));
    }
};

template <typename channels_type>
struct CompositeFunction<channels_type, KIS_MASKING_BRUSH_COMPOSITE_LINEAR_BURN, true> : public StrengthCompositeFunctionBase<channels_type>
{
    CompositeFunction(qreal strength) : StrengthCompositeFunctionBase<channels_type>(strength) {}
    
    channels_type apply(channels_type src, channels_type dst)
    {
        using namespace Arithmetic;
        using composite_type = typename KoColorSpaceMathsTraits<channels_type>::compositetype;
        return qBound(composite_type(KoColorSpaceMathsTraits<channels_type>::zeroValue),
                      composite_type(src) + mul(dst, StrengthCompositeFunctionBase<channels_type>::strength) - unitValue<channels_type>(),
                      composite_type(KoColorSpaceMathsTraits<channels_type>::unitValue));
    }
};

template <typename channels_type>
struct CompositeFunction<channels_type, KIS_MASKING_BRUSH_COMPOSITE_HARD_MIX_PHOTOSHOP, false>
{
    channels_type apply(channels_type src, channels_type dst)
    {
        return cfHardMixPhotoshop(src, dst);
    }
};

template <typename channels_type>
struct CompositeFunction<channels_type, KIS_MASKING_BRUSH_COMPOSITE_HARD_MIX_PHOTOSHOP, true> : public StrengthCompositeFunctionBase<channels_type>
{
    CompositeFunction(qreal strength) : StrengthCompositeFunctionBase<channels_type>(strength) {}
    
    channels_type apply(channels_type src, channels_type dst)
    {
        return cfHardMixPhotoshop(src, Arithmetic::mul(dst, StrengthCompositeFunctionBase<channels_type>::strength));
    }
};

template <typename channels_type>
struct CompositeFunction<channels_type, KIS_MASKING_BRUSH_COMPOSITE_HARD_MIX_SOFTER_PHOTOSHOP, false>
{
    channels_type apply(channels_type src, channels_type dst)
    {
        return cfHardMixSofterPhotoshop(src, dst);
    }
};

template <typename channels_type>
struct CompositeFunction<channels_type, KIS_MASKING_BRUSH_COMPOSITE_HARD_MIX_SOFTER_PHOTOSHOP, true> : public StrengthCompositeFunctionBase<channels_type>
{
    CompositeFunction(qreal strength) : StrengthCompositeFunctionBase<channels_type>(strength) {}
    
    channels_type apply(channels_type src, channels_type dst)
    {
        return cfHardMixSofterPhotoshop(src, Arithmetic::mul(dst, StrengthCompositeFunctionBase<channels_type>::strength));
    }
};

/**
 * A special Subtract variant for alpha channel.
 *
 * The meaning of alpha channel is a bit different from the one in color.
 * If the result of the subtraction becomes negative, we should clamp it
 * to the unit range. Otherwise, the layer may have negative alpha channel,
 * which generates funny artifacts :) See bug 424210.
 */
template <typename channels_type>
struct CompositeFunction<channels_type, KIS_MASKING_BRUSH_COMPOSITE_SUBTRACT, false>
{
    channels_type apply(channels_type src, channels_type dst)
    {
        using composite_type = typename KoColorSpaceMathsTraits<channels_type>::compositetype;
        using namespace Arithmetic;
        return qBound(composite_type(KoColorSpaceMathsTraits<channels_type>::zeroValue),
                      composite_type(dst) - src,
                      composite_type(KoColorSpaceMathsTraits<channels_type>::unitValue));
    }
};

template <typename channels_type>
struct CompositeFunction<channels_type, KIS_MASKING_BRUSH_COMPOSITE_SUBTRACT, true> : public StrengthCompositeFunctionBase<channels_type>
{
    const channels_type invertedStrength;

    CompositeFunction(qreal strength)
        : StrengthCompositeFunctionBase<channels_type>(strength)
        , invertedStrength(Arithmetic::inv(StrengthCompositeFunctionBase<channels_type>::strength))
    {}
    
    channels_type apply(channels_type src, channels_type dst)
    {
        using composite_type = typename KoColorSpaceMathsTraits<channels_type>::compositetype;
        using namespace Arithmetic;
        return qBound(composite_type(KoColorSpaceMathsTraits<channels_type>::zeroValue),
                      composite_type(dst) - (composite_type(src) + invertedStrength),
                      composite_type(KoColorSpaceMathsTraits<channels_type>::unitValue));
    }
};

template <typename channels_type>
struct CompositeFunction<channels_type, KIS_MASKING_BRUSH_COMPOSITE_HEIGHT, true> : public StrengthCompositeFunctionBase<channels_type>
{
    const channels_type invertedStrength;

    CompositeFunction(qreal strength)
        : StrengthCompositeFunctionBase<channels_type>(0.99 * strength)
        , invertedStrength(Arithmetic::inv(StrengthCompositeFunctionBase<channels_type>::strength))
    {}
    
    channels_type apply(channels_type src, channels_type dst)
    {
        using composite_type = typename KoColorSpaceMathsTraits<channels_type>::compositetype;
        using namespace Arithmetic;
        return qBound(composite_type(KoColorSpaceMathsTraits<channels_type>::zeroValue),
                      div(dst, invertedStrength) - (composite_type(src) + invertedStrength),
                      composite_type(KoColorSpaceMathsTraits<channels_type>::unitValue));
    }
};

template <typename channels_type>
struct CompositeFunction<channels_type, KIS_MASKING_BRUSH_COMPOSITE_LINEAR_HEIGHT, true> : public StrengthCompositeFunctionBase<channels_type>
{
    const channels_type invertedStrength;

    CompositeFunction(qreal strength)
        : StrengthCompositeFunctionBase<channels_type>(0.99 * strength)
        , invertedStrength(Arithmetic::inv(StrengthCompositeFunctionBase<channels_type>::strength))
    {}
    
    channels_type apply(channels_type src, channels_type dst)
    {
        using composite_type = typename KoColorSpaceMathsTraits<channels_type>::compositetype;
        using namespace Arithmetic;

        const composite_type modifiedDst = div(dst, invertedStrength) - invertedStrength;
        const composite_type multiply = modifiedDst * inv(src) / unitValue<channels_type>();
        const composite_type height = modifiedDst - src;
        return qBound(composite_type(KoColorSpaceMathsTraits<channels_type>::zeroValue),
                      qMax(multiply, height),
                      composite_type(KoColorSpaceMathsTraits<channels_type>::unitValue));
    }
};

template <typename channels_type>
struct CompositeFunction<channels_type, KIS_MASKING_BRUSH_COMPOSITE_HEIGHT_PHOTOSHOP, true> : public StrengthCompositeFunctionBase<channels_type>
{
    const typename KoColorSpaceMathsTraits<channels_type>::compositetype weight;

    CompositeFunction(qreal strength)
        : StrengthCompositeFunctionBase<channels_type>(strength)
        , weight(10 * StrengthCompositeFunctionBase<channels_type>::strength)
    {}
    
    channels_type apply(channels_type src, channels_type dst)
    {
        using composite_type = typename KoColorSpaceMathsTraits<channels_type>::compositetype;
        using namespace Arithmetic;
        return qBound(composite_type(KoColorSpaceMathsTraits<channels_type>::zeroValue),
                      dst * weight / unitValue<channels_type>() - src,
                      composite_type(KoColorSpaceMathsTraits<channels_type>::unitValue));
    }
};

template <typename channels_type>
struct CompositeFunction<channels_type, KIS_MASKING_BRUSH_COMPOSITE_LINEAR_HEIGHT_PHOTOSHOP, true> : public StrengthCompositeFunctionBase<channels_type>
{
    using composite_type = typename KoColorSpaceMathsTraits<channels_type>::compositetype;

    const composite_type weight;

    CompositeFunction(qreal strength)
        : StrengthCompositeFunctionBase<channels_type>(strength)
        , weight(10 * StrengthCompositeFunctionBase<channels_type>::strength)
    {}
    
    channels_type apply(channels_type src, channels_type dst)
    {
        using namespace Arithmetic;
        composite_type modifiedDst = dst * weight / unitValue<channels_type>();
        composite_type multiply = inv(src) * modifiedDst / unitValue<channels_type>();
        composite_type height = modifiedDst - src;
        return qBound(composite_type(KoColorSpaceMathsTraits<channels_type>::zeroValue),
                      qMax(multiply, height),
                      composite_type(KoColorSpaceMathsTraits<channels_type>::unitValue));
    }
};

}

template <typename channels_type, int composite_function, bool mask_is_alpha = false, bool use_strength = false>
class KisMaskingBrushCompositeOp : public KisMaskingBrushCompositeOpBase
{
public:
    using MaskPixel = typename std::conditional<mask_is_alpha, quint8, KoGrayU8Traits::Pixel>::type;

    template <bool use_strength_ = use_strength, typename = typename std::enable_if<!use_strength_>::type>
    KisMaskingBrushCompositeOp(int dstPixelSize, int dstAlphaOffset)
        : m_dstPixelSize(dstPixelSize)
        , m_dstAlphaOffset(dstAlphaOffset)
    {}

    template <bool use_strength_ = use_strength, typename = typename std::enable_if<use_strength_>::type>
    KisMaskingBrushCompositeOp(int dstPixelSize, int dstAlphaOffset, qreal strength)
        : m_dstPixelSize(dstPixelSize)
        , m_dstAlphaOffset(dstAlphaOffset)
        , m_compositeFunction(strength)
    {}

    void composite(const quint8 *srcRowStart, int srcRowStride,
                   quint8 *dstRowStart, int dstRowStride,
                   int columns, int rows) override
    {
        dstRowStart += m_dstAlphaOffset;

        for (int y = 0; y < rows; y++) {
            const quint8 *srcPtr = srcRowStart;
            quint8 *dstPtr = dstRowStart;

            for (int x = 0; x < columns; x++) {

                const MaskPixel *srcDataPtr = reinterpret_cast<const MaskPixel*>(srcPtr);

                const quint8 mask = preprocessMask(srcDataPtr);
                const channels_type maskScaled = KoColorSpaceMaths<quint8, channels_type>::scaleToA(mask);

                channels_type *dstDataPtr = reinterpret_cast<channels_type*>(dstPtr);
                *dstDataPtr = m_compositeFunction.apply(maskScaled, *dstDataPtr);

                srcPtr += sizeof(MaskPixel);
                dstPtr += m_dstPixelSize;
            }

            srcRowStart += srcRowStride;
            dstRowStart += dstRowStride;
        }
    }


private:
    inline quint8 preprocessMask(const quint8 *pixel)
    {
        return *pixel;
    }

    inline quint8 preprocessMask(const KoGrayU8Traits::Pixel *pixel)
    {
        return KoColorSpaceMaths<quint8>::multiply(pixel->gray, pixel->alpha);
    }

private:
    int m_dstPixelSize;
    int m_dstAlphaOffset;
    KisMaskingBrushCompositeDetail::CompositeFunction<channels_type, composite_function, use_strength> m_compositeFunction;
};

#endif // KISMASKINGBRUSHCOMPOSITEOP_H
