/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __VECTOR_MATH_H
#define __VECTOR_MATH_H

#include <Vc/Vc>
#include <Vc/IO>

#include <stdint.h>

#ifndef ALWAYS_INLINE
#if defined __GNUC__
#define ALWAYS_INLINE inline __attribute__((__always_inline__))
#elif defined _MSC_VER
#define ALWAYS_INLINE __forceinline
#else
#define ALWAYS_INLINE inline
#endif
#endif

template<Vc::Implementation _impl>
struct KoStreamedMath {

/**
 * Composes src into dst without using vector instructions
 */
template<bool useMask, bool useFlow, class Compositor>
    static void genericComposite32_novector(const KoCompositeOp::ParameterInfo& params)
{
    using namespace Arithmetic;

    const qint32 linearInc = 4;
    qint32 srcLinearInc = params.srcRowStride ? 4 : 0;

    quint8*       dstRowStart  = params.dstRowStart;
    const quint8* maskRowStart = params.maskRowStart;
    const quint8* srcRowStart  = params.srcRowStart;

    for(quint32 r=params.rows; r>0; --r) {
        const quint8 *mask = maskRowStart;
        const quint8 *src  = srcRowStart;
        quint8       *dst  = dstRowStart;

        int blockRest = params.cols;

        for(int i = 0; i < blockRest; i++) {
            Compositor::template compositeOnePixelScalar<useMask, _impl>(src, dst, mask, params.opacity, params.flow, params.channelFlags);
            src += srcLinearInc;
            dst += linearInc;

            if (useMask) {
                mask++;
            }
        }

        srcRowStart  += params.srcRowStride;
        dstRowStart  += params.dstRowStride;

        if (useMask) {
            maskRowStart += params.maskRowStride;
        }
    }
}

static inline quint8 lerp_mixed_u8_float(quint8 a, quint8 b, float alpha) {
    return quint8(qint16(b - a) * alpha + a);
}

/**
 * Get a vector containing first Vc::float_v::Size values of mask.
 * Each source mask element is considered to be a 8-bit integer
 */
static inline Vc::float_v fetch_mask_8(const quint8 *data) {
    Vc::uint_v data_i(data);
    return Vc::float_v(Vc::int_v(data_i));
}

/**
 * Get an alpha values from Vc::float_v::Size pixels 32-bit each
 * (4 channels, 8 bit per channel).  The alpha value is considered
 * to be stored in the most significat byte of the pixel
 *
 * \p aligned controls whether the \p data is fetched using aligned
 *            instruction or not.
 *            1) Fetching aligned data with unaligned instruction
 *               degrades performance.
 *            2) Fetching unaligned data with aligned instruction
 *               causes #GP (General Protection Exception)
 */
template <bool aligned>
static inline Vc::float_v fetch_alpha_32(const quint8 *data) {
    Vc::uint_v data_i;
    if (aligned) {
        data_i.load((const quint32*)data, Vc::Aligned);
    } else {
        data_i.load((const quint32*)data, Vc::Unaligned);
    }

    return Vc::float_v(Vc::int_v(data_i >> 24));
}

/**
 * Get color values from Vc::float_v::Size pixels 32-bit each
 * (4 channels, 8 bit per channel).  The color data is considered
 * to be stored in the 3 least significant bytes of the pixel.
 *
 * \p aligned controls whether the \p data is fetched using aligned
 *            instruction or not.
 *            1) Fetching aligned data with unaligned instruction
 *               degrades performance.
 *            2) Fetching unaligned data with aligned instruction
 *               causes #GP (General Protection Exception)
 */
template <bool aligned>
static inline void fetch_colors_32(const quint8 *data,
                            Vc::float_v &c1,
                            Vc::float_v &c2,
                            Vc::float_v &c3) {
    Vc::uint_v data_i;
    if (aligned) {
        data_i.load((const quint32*)data, Vc::Aligned);
    } else {
        data_i.load((const quint32*)data, Vc::Unaligned);
    }

    const quint32 lowByteMask = 0xFF;
    Vc::uint_v mask(lowByteMask);

    c1 = Vc::float_v(Vc::int_v((data_i >> 16) & mask));
    c2 = Vc::float_v(Vc::int_v((data_i >> 8)  & mask));
    c3 = Vc::float_v(Vc::int_v( data_i        & mask));
}

/**
 * Pack color and alpha values to Vc::float_v::Size pixels 32-bit each
 * (4 channels, 8 bit per channel).  The color data is considered
 * to be stored in the 3 least significant bytes of the pixel, alpha -
 * in the most significant byte
 *
 * NOTE: \p data must be aligned pointer!
 */
static inline void write_channels_32(quint8 *data,
                                     Vc::float_v::AsArg alpha,
                                     Vc::float_v::AsArg c1,
                                     Vc::float_v::AsArg c2,
                                     Vc::float_v::AsArg c3) {
    /**
     * FIXME: make conversion float->int
     * use methematical rounding
     */

    const quint32 lowByteMask = 0xFF;
    Vc::uint_v mask(lowByteMask);

    Vc::uint_v v1 = Vc::uint_v(Vc::int_v(alpha)) << 24;
    Vc::uint_v v2 = (Vc::uint_v(Vc::int_v(c1)) & mask) << 16;
    Vc::uint_v v3 = (Vc::uint_v(Vc::int_v(c2)) & mask) <<  8;
    v1 = v1 | v2;
    Vc::uint_v v4 = Vc::uint_v(Vc::int_v(c3)) & mask;
    v3 = v3 | v4;

    *((Vc::uint_v*)data) = v1 | v3;
}

/**
 * Composes src pixels into dst pixles. Is optimized for 32-bit-per-pixel
 * colorspaces. Uses \p Compositor strategy parameter for doing actual
 * math of the composition
 */
template<bool useMask, bool useFlow, class Compositor>
    static void genericComposite32(const KoCompositeOp::ParameterInfo& params)
{
    using namespace Arithmetic;

    const int vectorSize = Vc::float_v::Size;
    const qint32 vectorInc = 4 * vectorSize;
    const qint32 linearInc = 4;
    qint32 srcVectorInc = vectorInc;
    qint32 srcLinearInc = 4;

    quint8*       dstRowStart  = params.dstRowStart;
    const quint8* maskRowStart = params.maskRowStart;
    const quint8* srcRowStart  = params.srcRowStart;

    if (!params.srcRowStride) {
        quint32 *buf = Vc::malloc<quint32, Vc::AlignOnVector>(vectorSize);
        *((Vc::uint_v*)buf) = Vc::uint_v(*((const quint32*)params.srcRowStart));
        srcRowStart = reinterpret_cast<quint8*>(buf);
        srcLinearInc = 0;
        srcVectorInc = 0;
    }

    for(quint32 r=params.rows; r>0; --r) {
        // Hint: Mask is allowed to be unaligned
        const quint8 *mask = maskRowStart;

        const quint8 *src  = srcRowStart;
        quint8       *dst  = dstRowStart;

        const int pixelsAlignmentMask = vectorInc - 1;
        uintptr_t srcPtrValue = reinterpret_cast<uintptr_t>(src);
        uintptr_t dstPtrValue = reinterpret_cast<uintptr_t>(dst);
        uintptr_t srcAlignment = srcPtrValue & pixelsAlignmentMask;
        uintptr_t dstAlignment = dstPtrValue & pixelsAlignmentMask;

        // Uncomment if facing problems with alignment:
        // Q_ASSERT_X(!(dstAlignment & 3), "Compositioning",
        //            "Pixel data must be aligned on pixels borders!");

        int blockAlign = params.cols;
        int blockAlignedVector = 0;
        int blockUnalignedVector = 0;
        int blockRest = 0;

        int *vectorBlock =
            srcAlignment == dstAlignment || !srcVectorInc ?
            &blockAlignedVector : &blockUnalignedVector;

        if (!dstAlignment) {
            blockAlign = 0;
            *vectorBlock = params.cols / vectorSize;
            blockRest = params.cols % vectorSize;
        } else if (params.cols > 2 * vectorSize) {
            blockAlign = (vectorInc - dstAlignment) / 4;
            const int restCols = params.cols - blockAlign;
            *vectorBlock = restCols / vectorSize;
            blockRest = restCols % vectorSize;
        }

        for(int i = 0; i < blockAlign; i++) {
            Compositor::template compositeOnePixelScalar<useMask, _impl>(src, dst, mask, params.opacity, params.flow, params.channelFlags);
            src += srcLinearInc;
            dst += linearInc;

            if(useMask) {
                mask++;
            }
        }

        for (int i = 0; i < blockAlignedVector; i++) {
            Compositor::template compositeVector<useMask, true, _impl>(src, dst, mask, params.opacity, params.flow);
            src += srcVectorInc;
            dst += vectorInc;

            if (useMask) {
                mask += vectorSize;
            }
        }

        for (int i = 0; i < blockUnalignedVector; i++) {
            Compositor::template compositeVector<useMask, false, _impl>(src, dst, mask, params.opacity, params.flow);
            src += srcVectorInc;
            dst += vectorInc;

            if (useMask) {
                mask += vectorSize;
            }
        }


        for(int i = 0; i < blockRest; i++) {
            Compositor::template compositeOnePixelScalar<useMask, _impl>(src, dst, mask, params.opacity, params.flow, params.channelFlags);
            src += srcLinearInc;
            dst += linearInc;

            if (useMask) {
                mask++;
            }
        }

        srcRowStart  += params.srcRowStride;
        dstRowStart  += params.dstRowStride;

        if (useMask) {
            maskRowStart += params.maskRowStride;
        }
    }

    if (!params.srcRowStride) {
        Vc::free<float>(reinterpret_cast<float*>(const_cast<quint8*>(srcRowStart)));
    }
}
};

#endif /* __VECTOR_MATH_H */
