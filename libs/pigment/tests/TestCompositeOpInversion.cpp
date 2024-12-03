/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "TestCompositeOpInversion.h"
#include "KoColorModelStandardIds.h"

#include <cstring>

#include <simpletest.h>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoCompositeOpRegistry.h>
#include <KoBgrColorSpaceTraits.h>
#include <KoCmykColorSpaceTraits.h>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>

#include "kis_debug.h"

void TestCompositeOpInversion::test()
{
    /**
     * Comparing of CMYK against RGB would be more preferrable, but it just
     * doesn't work, the color space difference is too high. So we use an
     * alternative approach and just compare, which blend modes are invariant
     * to channel inversion.
     *
     * Basically, only few blendmodes are invariant to channel inversion:
     *
     *  - COMPOSITE_OVER
     *  - COMPOSITE_ALPHA_DARKEN;
     *  - COMPOSITE_COPY;
     *  - COMPOSITE_ERASE;
     *  - COMPOSITE_DESTINATION_IN;
     *  - COMPOSITE_DESTINATION_ATOP;
     *  - COMPOSITE_DISSOLVE;
     *
     *  All other blendmodes have too comlicated formulas (or alpha-composition),
     *  so they change their look on subtracted color spaces. Which means that
     *  all the blendmodes except the listed ones should be subjected to conversion
     *  to additive color space in CMYK mode.
     */

    QFETCH(QString, id);

    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();

    const KoCompositeOp *op = cs->compositeOp(id);

    KoColor srcColor(Qt::white, cs);
    KoColor dstColor(Qt::white, cs);
    KoColor origDstColor(Qt::white, cs);

    using namespace boost::accumulators;
    accumulator_set<qreal, stats<tag::max, tag::mean> > error;

    std::vector<int> alphaValues({16, 32, 64, 92, 128, 160, 192, 224, 255});
    std::vector<int> values({0, 16, 32, 64, 92, 128, 160, 192, 224, 255});

    Q_FOREACH (int opacity, alphaValues) {
        Q_FOREACH (int srcAlphaValue, alphaValues) {
            Q_FOREACH (int dstAlphaValue, alphaValues) {
                Q_FOREACH (int srcColorValue, values) {
                    Q_FOREACH (int dstColorValue, values) {
                        KoBgrU8Traits::Pixel *srcPixel = reinterpret_cast<KoBgrU8Traits::Pixel*>(srcColor.data());
                        KoBgrU8Traits::Pixel *dstPixel = reinterpret_cast<KoBgrU8Traits::Pixel*>(dstColor.data());
                        KoBgrU8Traits::Pixel *origDstPixel = reinterpret_cast<KoBgrU8Traits::Pixel*>(origDstColor.data());

                        srcPixel->red = srcColorValue;
                        srcPixel->green = 255 - srcColorValue;
                        srcPixel->alpha = srcAlphaValue;

                        dstPixel->red = dstColorValue;
                        dstPixel->green = 255 - dstColorValue;
                        dstPixel->alpha = dstAlphaValue;

                        std::memcpy(origDstColor.data(), dstColor.data(), cs->pixelSize());

                        const qreal opacityF = opacity / 255.0;
                        op->composite(dstColor.data(), 0, srcColor.data(), 0, 0, 0, 1, 1, opacityF);

                        dstPixel->green = 255 - dstPixel->green;

                        const int difference = dstPixel->red - dstPixel->green;

                        error(qAbs(difference));

                        if (qAbs(difference) > 1) {
                            //qDebug() << difference << srcColor << "+" << origDstColor << "->" << dstColor;
                        }
                    }
                }
            }
        }
    }

    qDebug() << op->id() << ppVar(max(error)) << ppVar(mean(error));
}

void TestCompositeOpInversion::test_data()
{
    QStringList ids;
    ids << COMPOSITE_OVER;
    ids << COMPOSITE_ALPHA_DARKEN;
    ids << COMPOSITE_COPY;
    ids << COMPOSITE_ERASE;
    ids << COMPOSITE_BEHIND;
    ids << COMPOSITE_DESTINATION_IN;
    ids << COMPOSITE_DESTINATION_ATOP;
    ids << COMPOSITE_GREATER;

    ids << COMPOSITE_OVERLAY;
    ids << COMPOSITE_GRAIN_MERGE;
    ids << COMPOSITE_GRAIN_EXTRACT;
    ids << COMPOSITE_HARD_MIX;
    ids << COMPOSITE_HARD_MIX_PHOTOSHOP;
    ids << COMPOSITE_HARD_MIX_SOFTER_PHOTOSHOP;
    ids << COMPOSITE_GEOMETRIC_MEAN;
    ids << COMPOSITE_PARALLEL;
    ids << COMPOSITE_ALLANON;
    ids << COMPOSITE_HARD_OVERLAY;
    ids << COMPOSITE_INTERPOLATION;
    ids << COMPOSITE_INTERPOLATIONB;
    ids << COMPOSITE_PENUMBRAA;
    ids << COMPOSITE_PENUMBRAB;
    ids << COMPOSITE_PENUMBRAC;
    ids << COMPOSITE_PENUMBRAD;
    ids << COMPOSITE_SCREEN;
    ids << COMPOSITE_DODGE;
    ids << COMPOSITE_LINEAR_DODGE;
    ids << COMPOSITE_LIGHTEN;
    ids << COMPOSITE_HARD_LIGHT;
    ids << COMPOSITE_SOFT_LIGHT_IFS_ILLUSIONS;
    ids << COMPOSITE_SOFT_LIGHT_PEGTOP_DELPHI;
    ids << COMPOSITE_SOFT_LIGHT_SVG;
    ids << COMPOSITE_SOFT_LIGHT_PHOTOSHOP;
    ids << COMPOSITE_GAMMA_LIGHT;
    ids << COMPOSITE_GAMMA_ILLUMINATION;
    ids << COMPOSITE_VIVID_LIGHT;
    ids << COMPOSITE_FLAT_LIGHT;
    ids << COMPOSITE_PIN_LIGHT;
    ids << COMPOSITE_LINEAR_LIGHT;
    ids << COMPOSITE_PNORM_A;
    ids << COMPOSITE_PNORM_B;
    ids << COMPOSITE_SUPER_LIGHT;
    ids << COMPOSITE_TINT_IFS_ILLUSIONS;
    ids << COMPOSITE_FOG_LIGHTEN_IFS_ILLUSIONS;
    ids << COMPOSITE_EASY_DODGE;
    ids << COMPOSITE_BURN;
    ids << COMPOSITE_LINEAR_BURN;
    ids << COMPOSITE_DARKEN;
    ids << COMPOSITE_GAMMA_DARK;
    ids << COMPOSITE_SHADE_IFS_ILLUSIONS;
    ids << COMPOSITE_FOG_DARKEN_IFS_ILLUSIONS;
    ids << COMPOSITE_EASY_BURN;
    ids << COMPOSITE_ADD;
    ids << COMPOSITE_SUBTRACT;
    ids << COMPOSITE_INVERSE_SUBTRACT;
    ids << COMPOSITE_MULT;
    ids << COMPOSITE_DIVIDE;
    ids << COMPOSITE_MOD;
    ids << COMPOSITE_MOD_CON;
    ids << COMPOSITE_DIVISIVE_MOD;
    ids << COMPOSITE_DIVISIVE_MOD_CON;
    ids << COMPOSITE_MODULO_SHIFT;
    ids << COMPOSITE_MODULO_SHIFT_CON;
    ids << COMPOSITE_ARC_TANGENT;
    ids << COMPOSITE_DIFF;
    ids << COMPOSITE_EXCLUSION;
    ids << COMPOSITE_EQUIVALENCE;
    ids << COMPOSITE_ADDITIVE_SUBTRACTIVE;
    ids << COMPOSITE_NEGATION;

    ids << COMPOSITE_XOR;
    ids << COMPOSITE_OR;
    ids << COMPOSITE_AND;
    ids << COMPOSITE_NAND;
    ids << COMPOSITE_NOR;
    ids << COMPOSITE_XNOR;
    ids << COMPOSITE_IMPLICATION;
    ids << COMPOSITE_NOT_IMPLICATION;
    ids << COMPOSITE_CONVERSE;
    ids << COMPOSITE_NOT_CONVERSE;

    ids << COMPOSITE_REFLECT;
    ids << COMPOSITE_GLOW;
    ids << COMPOSITE_FREEZE;
    ids << COMPOSITE_HEAT;
    ids << COMPOSITE_GLEAT;
    ids << COMPOSITE_HELOW;
    ids << COMPOSITE_REEZE;
    ids << COMPOSITE_FRECT;
    ids << COMPOSITE_FHYRD;

    ids << COMPOSITE_DISSOLVE;

    ids << COMPOSITE_LUMINOSITY_SAI;

    QTest::addColumn<QString>("id");

    Q_FOREACH (const QString &id, ids) {
        QTest::addRow("%s", id.toLatin1().data()) << id;
    }
}

#include <KoCompositeOpGeneric2.h>
#include <KoCompositeOpFunctions2.h>
#include "compositeops/KoColorSpaceBlendingPolicy.h"

#include <KoColorModelStandardIds.h>
void TestCompositeOpInversion::testBurnInF32Mode()
{
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), 0);
    const KoCompositeOp *op = cs->compositeOp(COMPOSITE_BURN);

    const qreal opacity = 0.25;
    KoColor srcColor(QColor(10,0,20,255), cs);
    KoColor dstColor(QColor(0,1,2,10), cs);

    op->composite(dstColor.data(), 0, srcColor.data(), 0,
                  0, 0,
                  1, 1,
                  opacity);

    ENTER_FUNCTION() << ppVar(srcColor) << ppVar(dstColor);

}

namespace {
struct Wrapper {
    KoColor color;
    int index = 0;
};

QDebug operator<<(QDebug debug, const Wrapper &w) {
    const KoColor &c = w.color;

    if (c.colorSpace()->colorDepthId() == Float32BitsColorDepthID) {
        const float *ptr = reinterpret_cast<const float*>(c.data());
        debug.nospace() << "(" << ptr[w.index] << ", " << ptr[3] << ")";
    } else if (c.colorSpace()->colorDepthId() == Integer16BitsColorDepthID) {
        using namespace Arithmetic;
        const quint16 *ptr = reinterpret_cast<const quint16*>(c.data());
        debug.nospace() << "(" << qreal(ptr[2 - w.index]) / unitValue<quint16>() << ", " << qreal(ptr[3]) / unitValue<quint16>() << ")";
    }

    return debug.space();
}

Wrapper dumpPixel(const KoColor &color) {
    return {color, 0};
}

float getColorValue(const KoColor &c) {
    float result = 777.0;

    if (c.colorSpace()->colorDepthId() == Float32BitsColorDepthID) {
        const float *ptr = reinterpret_cast<const float*>(c.data());
        result = ptr[0];
    } else if (c.colorSpace()->colorDepthId() == Integer16BitsColorDepthID) {
        using namespace Arithmetic;
        const quint16 *ptr = reinterpret_cast<const quint16*>(c.data());
        result = qreal(ptr[2]) / unitValue<quint16>();
    }

    return result;
}

quint16 getColorValueU(const KoColor &c) {
    KIS_ASSERT(c.colorSpace()->colorDepthId() == Integer16BitsColorDepthID);

    using namespace Arithmetic;
    const quint16 *ptr = reinterpret_cast<const quint16*>(c.data());
    return ptr[2];
}



const KoCompositeOp* createOp(const KoColorSpace *cs, const QString &id, bool isHDR)
{
    const KoCompositeOp *op = nullptr;

    if (id == COMPOSITE_DODGE) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            if (isHDR) {
                using func = CFColorDodge<float, ClampPolicyFloatHDR>;
                op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryLight());
            } else {
                using func = CFColorDodge<float, ClampPolicyFloatSDR>;
                op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryLight());
            }
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFColorDodge<quint16, ClampPolicyInteger>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryLight());
        }
#if 1
    } else if (id == COMPOSITE_BURN) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            if (isHDR) {
                using func = CFColorBurn<float, ClampPolicyFloatHDR>;
                op = new KoCompositeOpGenericSCFunctor<Traits, func,
                        KoAdditiveBlendingPolicy<Traits>>
                        (cs, id, KoCompositeOp::categoryDark());
            } else {
                using func = CFColorBurn<float, ClampPolicyFloatSDR>;
                op = new KoCompositeOpGenericSCFunctor<Traits, func,
                        KoAdditiveBlendingPolicy<Traits>>
                        (cs, id, KoCompositeOp::categoryDark());
            }
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFColorBurn<quint16, ClampPolicyInteger>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
        }
    } else if (id == COMPOSITE_LINEAR_BURN) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            if (isHDR) {
                using func = CFLinearBurn<float, ClampPolicyFloatHDR>;
                op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            } else {
                using func = CFLinearBurn<float, ClampPolicyFloatSDR>;
                op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            }
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFLinearBurn<quint16, ClampPolicyInteger>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
        }

    } else if (id == COMPOSITE_HARD_LIGHT) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            using func = CFHardLight<float>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFHardLight<quint16>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_SOFT_LIGHT_PHOTOSHOP) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            using func = CFSoftLight<float>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFSoftLight<quint16>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_SOFT_LIGHT_SVG) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            using func = CFSoftLightSvg<float>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFSoftLightSvg<quint16>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_VIVID_LIGHT) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            if (isHDR) {
                using func = CFVividLight<float, ClampPolicyFloatHDR>;
                op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            } else {
                using func = CFVividLight<float, ClampPolicyFloatSDR>;
                op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            }
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFVividLight<quint16, ClampPolicyInteger>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_HARD_OVERLAY) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            if (isHDR) {
                using func = CFHardOverlay<float, ClampPolicyFloatHDR>;
                op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            } else {
                using func = CFHardOverlay<float, ClampPolicyFloatSDR>;
                op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            }
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFHardOverlay<quint16, ClampPolicyInteger>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    // } else if (id == COMPOSITE_FLAT_LIGHT) {
    //     if (cs->colorDepthId() == Float32BitsColorDepthID) {
    //         using Traits = KoRgbF32Traits;
    //         constexpr auto func = &cfFlatLight<float>;
    //         op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
    //     } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
    //         using Traits = KoRgbU16Traits;
    //         constexpr auto func = &cfFlatLight<quint16>;
    //         op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
    //     }
    } else if (id == COMPOSITE_PIN_LIGHT) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            if (isHDR) {
                using func = CFPinLight<float, ClampPolicyFloatHDR>;
                op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            } else {
                using func = CFPinLight<float, ClampPolicyFloatSDR>;
                op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            }
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFPinLight<quint16, ClampPolicyInteger>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_ADD) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfAddition<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfAddition<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_SUBTRACT) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfSubtract<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfSubtract<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_INVERSE_SUBTRACT) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfInverseSubtract<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfInverseSubtract<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_MULT) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfMultiply<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfMultiply<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_DIVIDE) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfDivide<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfDivide<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_EXCLUSION) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfExclusion<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfExclusion<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_DIFF) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfDifference<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfDifference<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_SCREEN) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfScreen<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfScreen<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_DARKEN) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfDarkenOnly<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfDarkenOnly<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_LIGHTEN) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfLightenOnly<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfLightenOnly<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_NEGATION) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfNegation<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfNegation<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_HARD_MIX) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            if (isHDR) {
                using func = CFHardMix<float, ClampPolicyFloatHDR>;
                op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            } else {
                using func = CFHardMix<float, ClampPolicyFloatSDR>;
                op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            }
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFHardMix<quint16, ClampPolicyInteger>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_HARD_MIX_PHOTOSHOP) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfHardMixPhotoshop<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfHardMixPhotoshop<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_HARD_MIX_PHOTOSHOP) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfHardMixSofterPhotoshop<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfHardMixSofterPhotoshop<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_LUMINOSITY_SAI) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfAdditionSAI<HSVType, float>;
            op = new KoCompositeOpGenericSCAlpha<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfAdditionSAI<HSVType, float>;
            op = new KoCompositeOpGenericSCAlpha<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_ARC_TANGENT) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfArcTangent<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfArcTangent<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_ALLANON) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfAllanon<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfAllanon<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_PARALLEL) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            using func = CFParallel<float>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFParallel<quint16>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_EQUIVALENCE) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            using func = CFEquivalence<float>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFEquivalence<quint16>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_GRAIN_MERGE) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfGrainMerge<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfGrainMerge<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_GRAIN_EXTRACT) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfGrainExtract<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfGrainExtract<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_GEOMETRIC_MEAN) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            using func = CFGeometricMean<float>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFGeometricMean<quint16>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_ADDITIVE_SUBTRACTIVE) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            using func = CFAdditiveSubtractive<float>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFAdditiveSubtractive<quint16>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_GAMMA_DARK) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            using func = CFGammaDark<float>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFGammaDark<quint16>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_GAMMA_LIGHT) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            using func = CFGammaLight<float>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFGammaLight<quint16>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_GAMMA_ILLUMINATION) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            using func = CFGammaIllumination<float>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            using func = CFGammaIllumination<quint16>;
            op = new KoCompositeOpGenericSCFunctor<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
#endif
    } else if (id == COMPOSITE_DISSOLVE ||
               id == COMPOSITE_GREATER ||
               id == COMPOSITE_OVER ||
               id == COMPOSITE_ALPHA_DARKEN ||
               id == COMPOSITE_DESTINATION_ATOP ||
               id == COMPOSITE_DESTINATION_IN ||
               id == COMPOSITE_ERASE ||
               id == COMPOSITE_COPY) {
        op = cs->compositeOp(id);
    }

    KIS_ASSERT(op);

    return op;
}

}

enum TestFlag {
    None = 0x0,
    HDR = 0x1,
    UndefinedIfSrcInHDRRange = 0x2,
    AllowClampedComparisonInSDR = 0x4,
    SrcCannotMakeNegative = 0x8,
    ForceClippedResult = 0x10,
    UseStrictSdrRange = 0x20,
    PreservesSdrRange = 0x40,
    PreservesStrictSdrRange = 0x80
};
Q_DECLARE_FLAGS(TestFlags, TestFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(TestFlags)
Q_DECLARE_METATYPE(TestFlags)

void TestCompositeOpInversion::testFloatModes()
{
    QFETCH(QString, id);
    QFETCH(TestFlags, flags);

    const KoColorSpace* csU = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Integer16BitsColorDepthID.id(), 0);
    const KoCompositeOp *opU = createOp(csU, id, flags.testFlag(HDR));
    const KoCompositeOp *opU2 = csU->compositeOp(id);

    const KoColorSpace* csF = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), 0);
    const KoCompositeOp *opF = createOp(csF, id, flags.testFlag(HDR));

    auto createColorU = [&] (qreal colorF, qreal alphaF) {
        using namespace Arithmetic;

        /**
         * Some composite ops reply on the halfValue to decide about the
         * processing they do, so we should make sure that float and uint16
         * modes use the same half-point in the test
         */
        const quint16 color = qFuzzyCompare(colorF, 0.5) ?
                    KoColorSpaceMathsTraits<quint16>::halfValue :
                    qRound(qBound(0.0, colorF, 1.0) * unitValue<quint16>());
        const quint16 alpha = qRound(qBound(0.0, alphaF, 1.0) * unitValue<quint16>());

        KoColor c(csU);
        quint16 *ptr = reinterpret_cast<quint16*>(c.data());
        ptr[2] = color;
        ptr[3] = alpha;
        return c;
    };

    auto createColorF = [&] (qreal color, qreal alpha) {
        KoColor c(csF);
        float *ptr = reinterpret_cast<float*>(c.data());
        ptr[0] = color;
        ptr[3] = alpha;
        return c;
    };

    using namespace boost::accumulators;
    accumulator_set<qreal, stats<tag::max, tag::mean> > error;

    std::vector<qreal> opacityValues({0.0,
                                      0 + 2 * std::numeric_limits<float>::epsilon(),
                                      1.0f / 255.0f,
                                      0.1, 0.2, 0.5, 0.8, 0.9,
                                      1.0 - 1.0f / 255.0f,
                                      1.0 - 2 * std::numeric_limits<float>::epsilon(),
                                      1.0});
    //std::vector<qreal> opacityValues({1.0});
    //std::vector<qreal> alphaValues({1.0});
    std::vector<qreal> alphaValues = opacityValues;
    std::vector<qreal> values({-0.1,
                               0 - std::numeric_limits<float>::epsilon(),
                               0,
                               0 + std::numeric_limits<float>::epsilon(),
                               0.1, 0.2, 0.5, 0.8, 0.9,
                               1.0 - std::numeric_limits<float>::epsilon(),
                               1.0,
                               1.0 + std::numeric_limits<float>::epsilon(),
                               1.1,
                               1.5});
    std::vector<qreal> valuesSrc = values;
    std::vector<qreal> valuesDst = values;

//    std::vector<qreal> valuesSrc({0.0});
//    std::vector<qreal> valuesDst({-0.1});

    Q_FOREACH (qreal opacity, opacityValues) {
        Q_FOREACH (qreal srcAlphaValue, alphaValues) {
            Q_FOREACH (qreal dstAlphaValue, alphaValues) {
                Q_FOREACH (qreal srcColorValue, valuesSrc) {
                    Q_FOREACH (qreal dstColorValue, valuesDst) {

                        const qreal minOpacity = std::min({opacity, srcAlphaValue, dstAlphaValue});


                        float tolerance = minOpacity > 0.999 ? 0.001 :
                                          minOpacity > 0.5 ? 10 :
                                          minOpacity > 0.2 ? 100 :
                                          100;
//    {
//        {
//            {
//                {
//                    {
//                        qreal srcColorValue = 1.0 - std::numeric_limits<float>::epsilon();
//                        qreal dstColorValue = 0.2;

//                        qreal opacity = 0.5;
//                        qreal srcAlphaValue = 0.5;
//                        qreal dstAlphaValue = 0.5;

                        KoColor srcColorF = createColorF(srcColorValue,srcAlphaValue);
                        KoColor dstColorF = createColorF(dstColorValue,dstAlphaValue);
                        KoColor resultColorF = dstColorF;

                        opF->composite(resultColorF.data(), 0, srcColorF.data(), 0,
                                       0, 0,
                                       1, 1,
                                       opacity);

                        KoColor srcColorU = createColorU(srcColorValue,srcAlphaValue);
                        KoColor dstColorU = createColorU(dstColorValue,dstAlphaValue);
                        KoColor resultColorU = dstColorU;

                        opU->composite(resultColorU.data(), 0, srcColorU.data(), 0,
                                       0, 0,
                                       1, 1,
                                       opacity);

                        float resultColorValueU = getColorValue(resultColorU);
                        float resultColorValueF = getColorValue(resultColorF);


                        if (!flags.testFlag(HDR) &&
                                id != COMPOSITE_DISSOLVE &&
                                id != COMPOSITE_GREATER &&
                                id != COMPOSITE_OVER &&
                                id != COMPOSITE_ALPHA_DARKEN &&
                                id != COMPOSITE_DESTINATION_ATOP &&
                                id != COMPOSITE_DESTINATION_IN &&
                                id != COMPOSITE_ERASE &&
                                id != COMPOSITE_COPY &&
                                !qFuzzyIsNull(float(minOpacity))) { // we have added a fast-path for opacity == 0.0 case
                            KoColor resultColorU2 = dstColorU;

                            opU2->composite(resultColorU2.data(), 0, srcColorU.data(), 0,
                                            0, 0,
                                            1, 1,
                                            opacity);

                            const quint16 referenceColor = getColorValueU(resultColorU2);
                            const quint16 resultColor = getColorValueU(resultColorU);

                            const int difference = qAbs(referenceColor - resultColor);
                            int maxDifference = 0;

                            /**
                             * Hard overlay was changed from float to int16 composition
                             */
                            if (id == COMPOSITE_HARD_OVERLAY) {
                                maxDifference = minOpacity > 0.99 ? 1 : 10;
                            }

                            /**
                             * We have changed CompositeGeneric so now there is a precise
                             * case for alpha-unity values
                             */
                            if (qFuzzyCompare(float(dstAlphaValue), 1.0f) ||
                                    qFuzzyCompare(float(srcAlphaValue), 1.0f)) {
                                maxDifference = 3;
                            }

                            /**
                             * We have added a static point that resolves numerical
                             * instability.
                             */
                            const bool shouldSkipCheck =
                                    (id == COMPOSITE_HARD_LIGHT &&
                                    qFuzzyCompare(srcColorValue, 0.5));

                            if (!shouldSkipCheck && difference > maxDifference) {

                                qDebug() << "--- integer implementation is inconsistent to the original mode! ---";
                                qDebug() << ppVar(opacity) << ppVar(resultColor) << ppVar(referenceColor);
                                qDebug() << "U16 result:   " << fixed << qSetRealNumberPrecision(8)
                                         << "s:" << dumpPixel(srcColorU) << "+" << "d:" << dumpPixel(dstColorU) << "->" << dumpPixel(resultColorU);
                                qDebug() << "U16 reference:" << fixed << qSetRealNumberPrecision(8)
                                         << "s:" << dumpPixel(srcColorU) << "+" << "d:" << dumpPixel(dstColorU) << "->" << dumpPixel(resultColorU2);
                                QFAIL("integer implementation is inconsistent to the original mode!");
                            }
                        }

                        if (std::isnan(resultColorValueF)) {
                            qDebug() << "--- NaN value is found! ---";
                            qDebug() << ppVar(opacity);
                            qDebug() << "U16:" << fixed << qSetRealNumberPrecision(8)
                                     << "s:" << dumpPixel(srcColorU) << "+" << "d:" << dumpPixel(dstColorU) << "->" << dumpPixel(resultColorU);
                            qDebug() << "F32:" << fixed << qSetRealNumberPrecision(8)
                                     << "s:" << dumpPixel(srcColorF) << "+" << "d:" << dumpPixel(dstColorF) << "->" << dumpPixel(resultColorF);
                            QFAIL("NaN value is found!");
                        }

                        QVERIFY(!std::isinf(resultColorValueF));

                        auto checkInSdrRangeImpl = [=] (qreal value, bool useStrictRange) {
                            return useStrictRange ?
                                        (value >= 0.0 && value <= 1.0) :
                                        (value >= -std::numeric_limits<float>::epsilon() &&
                                         value <= 1.0 + std::numeric_limits<float>::epsilon());
                        };

                        auto checkInSdrRange = [=] (qreal value) {
                            return checkInSdrRangeImpl(value, flags.testFlag(UseStrictSdrRange));
                        };

                        const bool isInSdrRange =
                                checkInSdrRange(dstColorValue) &&
                                checkInSdrRange(srcColorValue);

                        if (isInSdrRange &&
                            qAbs(resultColorValueF - resultColorValueU) > tolerance) {

                            if (qFuzzyIsNull(resultColorValueU) &&
                                    qAbs(resultColorValueU - qMax(0.0f, resultColorValueF)) <= 0.001) {
                                // noop, should be fine result
                            } else if (qFuzzyCompare(resultColorValueU, 1.0f) &&
                                       qAbs(resultColorValueU - qMin(1.0f, resultColorValueF)) <= 0.001) {
                                // noop, should be fine result as well
                            } else if (resultColorValueF < 0.0) {
                                qDebug() << "--- resulting value in SDR range is negative! ---";
                                qDebug() << ppVar(opacity);
                                qDebug() << "U16:" << fixed << qSetRealNumberPrecision(8)
                                         << "s:" << dumpPixel(srcColorU) << "+" << "d:" << dumpPixel(dstColorU) << "->" << dumpPixel(resultColorU);
                                qDebug() << "F32:" << fixed << qSetRealNumberPrecision(8)
                                         << "s:" << dumpPixel(srcColorF) << "+" << "d:" << dumpPixel(dstColorF) << "->" << dumpPixel(resultColorF);
                                QFAIL("resulting value in SDR range is negative!");
                            } else {

                                // in non-HDR mode both blending modes should behave in
                                // exactly the same way (including zero and unit values)

                                qDebug() << "--- inconsistent result in SDR range ---";
                                qDebug() << ppVar(opacity);
                                qDebug() << "U16:" << fixed << qSetRealNumberPrecision(8)
                                         << "s:" << dumpPixel(srcColorU) << "+" << "d:" << dumpPixel(dstColorU) << "->" << dumpPixel(resultColorU);
                                qDebug() << "F32:" << fixed << qSetRealNumberPrecision(8)
                                         << "s:" << dumpPixel(srcColorF) << "+" << "d:" << dumpPixel(dstColorF) << "->" << dumpPixel(resultColorF);
                                QFAIL("inconsistent result in SDR range!");
                            }
                        }

                        if ((flags.testFlag(PreservesSdrRange) ||
                             flags.testFlag(PreservesStrictSdrRange)) &&
                                checkInSdrRangeImpl(srcColorValue, flags.testFlag(PreservesStrictSdrRange)) &&
                                checkInSdrRangeImpl(dstColorValue, flags.testFlag(PreservesStrictSdrRange)) &&
                                !checkInSdrRangeImpl(resultColorValueF, flags.testFlag(PreservesStrictSdrRange))) {


                            bool resultConvergedToAPoint = false;

                            std::vector<float> transitionalValues;
                            transitionalValues.reserve(20 + 1);

                            for (int i = 0; i < 30; i++) {
                                transitionalValues.push_back(resultColorValueF);

                                opF->composite(resultColorF.data(), 0, srcColorF.data(), 0,
                                               0, 0,
                                               1, 1,
                                               opacity);
                                resultColorValueF = getColorValue(resultColorF);

                                if (std::isinf(resultColorValueF)) {
                                    qDebug() << ppVar(i);
                                    qDebug() << fixed << qSetRealNumberPrecision(16) << ppVar(srcColorValue) << ppVar(dstColorValue);
                                    qDebug() << fixed << qSetRealNumberPrecision(16) << ppVar(srcAlphaValue) << ppVar(dstAlphaValue);
                                }

                                QVERIFY(!std::isnan(resultColorValueF));
                                QVERIFY(!std::isinf(resultColorValueF));

                                /**
                                 * We are using strict comparison here explicitly, not
                                 * qFuzzyCompare(), because we need the algorithm to converge
                                 * strictly to a single value.
                                 */
                                if (resultColorValueF == transitionalValues.back()) {
                                    resultConvergedToAPoint = true;
                                    transitionalValues.push_back(resultColorValueF);
                                    break;
                                }
                            }

                            const float originalError = qAbs(transitionalValues.front() - resultColorValueU);
                            const float finalError = qAbs(transitionalValues.back() - resultColorValueU);

                            bool skipConvergencyCheck = false;

                            if ((id == COMPOSITE_DARKEN ||
                                 id == COMPOSITE_LIGHTEN ||
                                 id == COMPOSITE_ALLANON) &&
                                    qFuzzyCompare(float(srcColorValue), 1.0f) &&
                                    qFuzzyCompare(float(srcColorValue), 1.0f)) {

                                skipConvergencyCheck = true;
                            }

                            if (checkInSdrRangeImpl(transitionalValues.back(), true) ||

                                    ((resultConvergedToAPoint ||
                                      finalError < originalError ||
                                      skipConvergencyCheck) &&

                                     qAbs(transitionalValues.back() - transitionalValues.front())
                                     < 64 * std::numeric_limits<float>::epsilon())) {

                                // noop, everything is fine
                            } else {
                                qDebug() << "--- op does not preserve SDR range! ---";
                                qDebug() << fixed << qSetRealNumberPrecision(8)
                                         << ppVar(opacity) << ppVar(srcColorValue) << ppVar(dstColorValue) << ppVar(resultColorValueF)
                                         << fixed << qSetRealNumberPrecision(14);

                                qDebug() << "stabilization track:";
                                for (size_t i = 0; i < transitionalValues.size(); i++) {
                                    qDebug() << fixed << qSetRealNumberPrecision(14)
                                             << "    " << i << ":" << transitionalValues[i];
                                }

                                qDebug() << "U16:" << fixed << qSetRealNumberPrecision(8)
                                         << "s:" << dumpPixel(srcColorU) << "+" << "d:" << dumpPixel(dstColorU) << "->" << dumpPixel(resultColorU);
                                qDebug() << "F32:" << fixed << qSetRealNumberPrecision(8)
                                         << "s:" << dumpPixel(srcColorF) << "+" << "d:" << dumpPixel(dstColorF) << "->" << dumpPixel(resultColorF);

                                QFAIL("op does not preserve SDR range!");
                            }
                        }

                        if (flags.testFlag(SrcCannotMakeNegative) &&
                                dstColorValue >= 0.0f &&
                                dstColorValue <= 1.0f &&
                                resultColorValueF < -std::numeric_limits<float>::epsilon()) {

                            qDebug() << "--- resulting value in SDR range is negative for SRC-clipped op! ---";
                            qDebug() << ppVar(opacity);
                            qDebug() << "U16:" << fixed << qSetRealNumberPrecision(8)
                                     << "s:" << dumpPixel(srcColorU) << "+" << "d:" << dumpPixel(dstColorU) << "->" << dumpPixel(resultColorU);
                            qDebug() << "F32:" << fixed << qSetRealNumberPrecision(8)
                                     << "s:" << dumpPixel(srcColorF) << "+" << "d:" << dumpPixel(dstColorF) << "->" << dumpPixel(resultColorF);
                            QFAIL("resulting value in SDR range is negative for SRC-clipped op!");
                        }

                        // TODO; it doesn't work for some reason
                        if (opacity > 0.01 &&
                            flags.testFlag(ForceClippedResult) &&
                            resultColorValueF > 1.0f) {

                            qDebug() << "--- resulting value is outside SDR range for clipped op! ---";
                            qDebug() << ppVar(opacity);
                            qDebug() << "U16:" << fixed << qSetRealNumberPrecision(8)
                                     << "s:" << dumpPixel(srcColorU) << "+" << "d:" << dumpPixel(dstColorU) << "->" << dumpPixel(resultColorU);
                            qDebug() << "F32:" << fixed << qSetRealNumberPrecision(8)
                                     << "s:" << dumpPixel(srcColorF) << "+" << "d:" << dumpPixel(dstColorF) << "->" << dumpPixel(resultColorF);
                            QFAIL("resulting value in outside SDR range for clipped op!");
                        }
                    }
                }
            }
        }
    }
}

void TestCompositeOpInversion::testFloatModes_data()
{
    QStringList ids;
//    ids << COMPOSITE_OVER;
//    ids << COMPOSITE_ALPHA_DARKEN;
//    ids << COMPOSITE_COPY;
//    ids << COMPOSITE_ERASE;
//    ids << COMPOSITE_BEHIND;
//    ids << COMPOSITE_DESTINATION_IN;
//    ids << COMPOSITE_DESTINATION_ATOP;


    ids << COMPOSITE_HARD_MIX;

//    ids << COMPOSITE_OVERLAY; // equals to hard-light
//    ids << COMPOSITE_GRAIN_MERGE;
//    ids << COMPOSITE_GRAIN_EXTRACT;
//    ids << COMPOSITE_GEOMETRIC_MEAN;
//    ids << COMPOSITE_ALLANON;
    ids << COMPOSITE_HARD_OVERLAY;
//    ids << COMPOSITE_INTERPOLATION;
//    ids << COMPOSITE_INTERPOLATIONB;
//    ids << COMPOSITE_PENUMBRAA;
//    ids << COMPOSITE_PENUMBRAB;
//    ids << COMPOSITE_PENUMBRAC;
//    ids << COMPOSITE_PENUMBRAD;
//    ids << COMPOSITE_SCREEN;
    ids << COMPOSITE_DODGE;
//    ids << COMPOSITE_LINEAR_DODGE;
//    ids << COMPOSITE_LIGHTEN;
//    ids << COMPOSITE_SOFT_LIGHT_IFS_ILLUSIONS;
//    ids << COMPOSITE_SOFT_LIGHT_PEGTOP_DELPHI;
//    ids << COMPOSITE_GAMMA_LIGHT;
//    ids << COMPOSITE_GAMMA_ILLUMINATION;
    ids << COMPOSITE_VIVID_LIGHT;
    ids << COMPOSITE_PIN_LIGHT;
//    ids << COMPOSITE_LINEAR_LIGHT;
//    ids << COMPOSITE_PNORM_A;
//    ids << COMPOSITE_PNORM_B;
//    ids << COMPOSITE_SUPER_LIGHT;
//    ids << COMPOSITE_TINT_IFS_ILLUSIONS;
//    ids << COMPOSITE_FOG_LIGHTEN_IFS_ILLUSIONS;
//    ids << COMPOSITE_EASY_DODGE;
    ids << COMPOSITE_BURN;
    ids << COMPOSITE_LINEAR_BURN;
//    ids << COMPOSITE_DARKEN;
//    ids << COMPOSITE_GAMMA_DARK;
//    ids << COMPOSITE_SHADE_IFS_ILLUSIONS;
//    ids << COMPOSITE_FOG_DARKEN_IFS_ILLUSIONS;
//    ids << COMPOSITE_EASY_BURN;
//    ids << COMPOSITE_MOD;
//    ids << COMPOSITE_MOD_CON;
//    ids << COMPOSITE_DIVISIVE_MOD;
//    ids << COMPOSITE_DIVISIVE_MOD_CON;
//    ids << COMPOSITE_MODULO_SHIFT;
//    ids << COMPOSITE_MODULO_SHIFT_CON;
//    ids << COMPOSITE_DIFF;
//    ids << COMPOSITE_EXCLUSION;
//    ids << COMPOSITE_EQUIVALENCE;
//    ids << COMPOSITE_ADDITIVE_SUBTRACTIVE;
//    ids << COMPOSITE_NEGATION;

//    ids << COMPOSITE_XOR;
//    ids << COMPOSITE_OR;
//    ids << COMPOSITE_AND;
//    ids << COMPOSITE_NAND;
//    ids << COMPOSITE_NOR;
//    ids << COMPOSITE_XNOR;
//    ids << COMPOSITE_IMPLICATION;
//    ids << COMPOSITE_NOT_IMPLICATION;
//    ids << COMPOSITE_CONVERSE;
//    ids << COMPOSITE_NOT_CONVERSE;

//    ids << COMPOSITE_REFLECT;
//    ids << COMPOSITE_GLOW;
//    ids << COMPOSITE_FREEZE;
//    ids << COMPOSITE_HEAT;
//    ids << COMPOSITE_GLEAT;
//    ids << COMPOSITE_HELOW;
//    ids << COMPOSITE_REEZE;
//    ids << COMPOSITE_FRECT;
//    ids << COMPOSITE_FHYRD;




    QStringList sdrPreservingIds;

    sdrPreservingIds << COMPOSITE_DARKEN;
    sdrPreservingIds << COMPOSITE_LIGHTEN;
    sdrPreservingIds << COMPOSITE_ALLANON;
    sdrPreservingIds << COMPOSITE_DISSOLVE;
    sdrPreservingIds << COMPOSITE_OVER;
    sdrPreservingIds << COMPOSITE_ALPHA_DARKEN;
    sdrPreservingIds << COMPOSITE_DESTINATION_IN;
    sdrPreservingIds << COMPOSITE_DESTINATION_ATOP;
    sdrPreservingIds << COMPOSITE_ERASE;
    sdrPreservingIds << COMPOSITE_COPY;

    QStringList strictSdrPreservingIds;
    strictSdrPreservingIds << COMPOSITE_MULT;
    strictSdrPreservingIds << COMPOSITE_DIFF;
    strictSdrPreservingIds << COMPOSITE_SCREEN;
    strictSdrPreservingIds << COMPOSITE_GREATER;


    QStringList specialIds;
    // unclamped non-preserving blendmodes
    specialIds << COMPOSITE_ADD;
    specialIds << COMPOSITE_SUBTRACT;
    specialIds << COMPOSITE_INVERSE_SUBTRACT;
    specialIds << COMPOSITE_DIVIDE;
    specialIds << COMPOSITE_LUMINOSITY_SAI;
    specialIds << COMPOSITE_EXCLUSION;
    specialIds << COMPOSITE_NEGATION;
    specialIds << COMPOSITE_GRAIN_MERGE;
    specialIds << COMPOSITE_GRAIN_EXTRACT;

    QTest::addColumn<QString>("id");
    QTest::addColumn<TestFlags>("flags");

    auto fixId = [] (QString id) {
        id.replace(' ', '_');
        return id;
    };

    std::vector<bool> isHDRValues({false, true});

    Q_FOREACH (const QString &id, ids) {
        Q_FOREACH (const bool isHDR, isHDRValues) {
            TestFlags flags = isHDR ? HDR : None;
            flags |= SrcCannotMakeNegative;

            if (!isHDR) {
                flags |= PreservesSdrRange;
            } else {
                // TODO: not all blendmodes currently preserve SDR range in HDR mode
                //flags |= PreservesStrictSdrRange;
            }
            QTest::addRow("%s_%s", fixId(id).toLatin1().data(), isHDR ? "hdr" : "sdr") << id << flags;
        }
    }

    Q_FOREACH (const QString &id, specialIds) {
        TestFlags flags = None;
        QTest::addRow("%s_%s", fixId(id).toLatin1().data(), "sdr") << id << flags;
    }

    Q_FOREACH (const QString &id, sdrPreservingIds) {
        TestFlags flags = PreservesSdrRange;
        QTest::addRow("%s_%s", fixId(id).toLatin1().data(), "sdr") << id << flags;
    }

    Q_FOREACH (const QString &id, strictSdrPreservingIds) {
        TestFlags flags = PreservesStrictSdrRange;
        QTest::addRow("%s_%s", fixId(id).toLatin1().data(), "sdr") << id << flags;
    }

    // both channels are clamped, sdr only!
    QTest::addRow("%s_%s", COMPOSITE_SOFT_LIGHT_SVG.toLatin1().data(), "sdr") << COMPOSITE_SOFT_LIGHT_SVG << TestFlags(SrcCannotMakeNegative | PreservesSdrRange);
    QTest::addRow("%s_%s", COMPOSITE_SOFT_LIGHT_PHOTOSHOP.toLatin1().data(), "sdr") << COMPOSITE_SOFT_LIGHT_PHOTOSHOP << TestFlags(SrcCannotMakeNegative | PreservesSdrRange);

    QTest::addRow("%s_%s", COMPOSITE_HARD_MIX_PHOTOSHOP.toLatin1().data(), "sdr") << COMPOSITE_SOFT_LIGHT_SVG << TestFlags(SrcCannotMakeNegative | UseStrictSdrRange | PreservesStrictSdrRange);
    QTest::addRow("%s_%s", COMPOSITE_HARD_MIX_SOFTER_PHOTOSHOP.toLatin1().data(), "sdr") << COMPOSITE_SOFT_LIGHT_SVG << TestFlags(SrcCannotMakeNegative | UseStrictSdrRange | PreservesStrictSdrRange);
    QTest::addRow("%s_%s", COMPOSITE_ARC_TANGENT.toLatin1().data(), "sdr") << COMPOSITE_ARC_TANGENT << TestFlags(UseStrictSdrRange | PreservesStrictSdrRange);


    // doesn't clamp result, always preserves SDR state
    QTest::addRow("%s_%s", COMPOSITE_HARD_LIGHT.toLatin1().data(), "sdr") << COMPOSITE_HARD_LIGHT << TestFlags(SrcCannotMakeNegative | PreservesStrictSdrRange);
    QTest::addRow("%s_%s", COMPOSITE_PARALLEL.toLatin1().data(), "sdr") << COMPOSITE_PARALLEL << TestFlags(SrcCannotMakeNegative | PreservesSdrRange);
    QTest::addRow("%s_%s", COMPOSITE_EQUIVALENCE.toLatin1().data(), "sdr") << COMPOSITE_EQUIVALENCE << TestFlags(SrcCannotMakeNegative | PreservesStrictSdrRange);
    QTest::addRow("%s_%s", COMPOSITE_GEOMETRIC_MEAN.toLatin1().data(), "sdr") << COMPOSITE_GEOMETRIC_MEAN << TestFlags(SrcCannotMakeNegative | PreservesSdrRange);
    QTest::addRow("%s_%s", COMPOSITE_ADDITIVE_SUBTRACTIVE.toLatin1().data(), "sdr") << COMPOSITE_ADDITIVE_SUBTRACTIVE << TestFlags(SrcCannotMakeNegative | PreservesSdrRange);
    QTest::addRow("%s_%s", COMPOSITE_GAMMA_DARK.toLatin1().data(), "sdr") << COMPOSITE_GAMMA_DARK << TestFlags(SrcCannotMakeNegative | PreservesStrictSdrRange);
    QTest::addRow("%s_%s", COMPOSITE_GAMMA_LIGHT.toLatin1().data(), "sdr") << COMPOSITE_GAMMA_LIGHT << TestFlags(SrcCannotMakeNegative | PreservesSdrRange);
    QTest::addRow("%s_%s", COMPOSITE_GAMMA_ILLUMINATION.toLatin1().data(), "sdr") << COMPOSITE_GAMMA_ILLUMINATION << TestFlags(SrcCannotMakeNegative | PreservesStrictSdrRange);
}

SIMPLE_TEST_MAIN(TestCompositeOpInversion)
