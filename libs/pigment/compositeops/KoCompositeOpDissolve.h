/*
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _KOCOMPOSITEOP_DISSOLVE_H_
#define _KOCOMPOSITEOP_DISSOLVE_H_

#include <KoColorSpaceMaths.h>
#include <KoCompositeOp.h>

#include <kis_random_source.h>
#include <QThreadStorage>

template<class Traits>
class KoCompositeOpDissolve: public KoCompositeOp
{
    typedef typename Traits::channels_type channels_type;

    static const qint32 channels_nb = Traits::channels_nb;
    static const qint32 alpha_pos   = Traits::alpha_pos;

    mutable QThreadStorage<KisRandomSource> m_randomGenerator;

public:
    KoCompositeOpDissolve(const KoColorSpace* cs, const QString& category)
        : KoCompositeOp(cs, COMPOSITE_DISSOLVE, category)
    {
    }

    using KoCompositeOp::composite;

    void composite(const KoCompositeOp::ParameterInfo& params) const override {
        /**
         * Initialize local per-thread random generator from the global one, it will
         * reduce contestion over the global generator in the main compositing loop.
         *
         * KisRandomSource is automatically initialized from QRandomGenerator::global()
         * internally.
         */
        if (!m_randomGenerator.hasLocalData()) {
            m_randomGenerator.setLocalData(KisRandomSource());
        }

        const QBitArray& flags       = params.channelFlags.isEmpty() ? QBitArray(channels_nb,true) : params.channelFlags;
        bool             alphaLocked = (alpha_pos != -1) && !flags.testBit(alpha_pos);

        using namespace Arithmetic;

//         quint32       ctr       = quint32(reinterpret_cast<quint64>(dstRowStart) % 256);
        qint32        srcInc    = (params.srcRowStride == 0) ? 0 : channels_nb;
        bool          useMask   = params.maskRowStart != 0;
        channels_type unitValue = KoColorSpaceMathsTraits<channels_type>::unitValue;
        channels_type opacity   = KoColorSpaceMaths<float,channels_type>::scaleToA(params.opacity);

        const quint8 *srcRowStart = params.srcRowStart;
        quint8 *dstRowStart = params.dstRowStart;
        const quint8 *maskRowStart = params.maskRowStart;

        qint32 rows = params.rows;

        for(; rows>0; --rows) {
            const channels_type* src  = reinterpret_cast<const channels_type*>(srcRowStart);
            channels_type*       dst  = reinterpret_cast<channels_type*>(dstRowStart);
            const quint8*        mask = maskRowStart;

            for(qint32 c=params.cols; c>0; --c) {
                channels_type srcAlpha = (alpha_pos == -1) ? unitValue : src[alpha_pos];
                channels_type dstAlpha = (alpha_pos == -1) ? unitValue : dst[alpha_pos];
                channels_type blend    = useMask ? mul(opacity, scale<channels_type>(*mask), srcAlpha) : mul(opacity, srcAlpha);

                if (m_randomGenerator.localData().generate(0, 255) <= scale<quint8>(blend)
                    && blend != KoColorSpaceMathsTraits<channels_type>::zeroValue) {
                    for(qint32 i=0; i <channels_nb; i++) {
                        if(i != alpha_pos && flags.testBit(i))
                            dst[i] = src[i];
                    }

                    if(alpha_pos != -1)
                        dst[alpha_pos] = alphaLocked ? dstAlpha : unitValue;
                }

                src += srcInc;
                dst += channels_nb;
//                 ctr  = (ctr + 1) % 256;
                if (mask) {
                    ++mask;
                }
            }

            srcRowStart  += params.srcRowStride;
            dstRowStart  += params.dstRowStride;
            if (maskRowStart) {
                maskRowStart += params.maskRowStride;
            }
        }
    }
};

#endif // _KOCOMPOSITEOP_DISSOLVE_H_
