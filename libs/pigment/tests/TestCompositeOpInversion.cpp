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



KoCompositeOp* createOp(const KoColorSpace *cs, const QString &id, bool isHDR)
{
    KoCompositeOp *op = nullptr;

    if (id == COMPOSITE_DODGE) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            if (isHDR) {
                constexpr auto func = &cfColorDodge<float, ClampPolicyFloatHDR>;
                op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryLight());
            } else {
                constexpr auto func = &cfColorDodge<float, ClampPolicyFloatSDR>;
                op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryLight());
            }
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfColorDodge<quint16, ClampPolicyInteger>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryLight());
        }
    } else if (id == COMPOSITE_BURN) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            if (isHDR) {
                constexpr auto func = &cfColorBurn<float, ClampPolicyFloatHDR>;
                op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            } else {
                constexpr auto func = &cfColorBurn<float, ClampPolicyFloatSDR>;
                op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            }
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfColorBurn<quint16, ClampPolicyInteger>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
        }
    } else if (id == COMPOSITE_LINEAR_BURN) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            if (isHDR) {
                constexpr auto func = &cfLinearBurn<float, ClampPolicyFloatHDR>;
                op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            } else {
                constexpr auto func = &cfLinearBurn<float, ClampPolicyFloatSDR>;
                op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            }
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfLinearBurn<quint16, ClampPolicyInteger>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
        }

    } else if (id == COMPOSITE_HARD_LIGHT) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            if (isHDR) {
                constexpr auto func = &cfHardLight<float, ClampPolicyFloatHDR>;
                op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            } else {
                constexpr auto func = &cfHardLight<float, ClampPolicyFloatSDR>;
                op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            }
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfHardLight<quint16, ClampPolicyInteger>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_SOFT_LIGHT_PHOTOSHOP) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfSoftLight<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfSoftLight<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_SOFT_LIGHT_SVG) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            constexpr auto func = &cfSoftLightSvg<float>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfSoftLightSvg<quint16>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
        }
    } else if (id == COMPOSITE_VIVID_LIGHT) {
        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            using Traits = KoRgbF32Traits;
            if (isHDR) {
                constexpr auto func = &cfVividLight<float, ClampPolicyFloatHDR>;
                op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            } else {
                constexpr auto func = &cfVividLight<float, ClampPolicyFloatSDR>;
                op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            }
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfVividLight<quint16, ClampPolicyInteger>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
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
                constexpr auto func = &cfPinLight<float, ClampPolicyFloatHDR>;
                op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            } else {
                constexpr auto func = &cfPinLight<float, ClampPolicyFloatSDR>;
                op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryDark());
            }
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            using Traits = KoRgbU16Traits;
            constexpr auto func = &cfPinLight<quint16, ClampPolicyInteger>;
            op = new KoCompositeOpGenericSC<Traits, func, KoAdditiveBlendingPolicy<Traits>>(cs, id, KoCompositeOp::categoryArithmetic());
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
    ForceClippedResult = 0x10
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
        const quint16 color = qRound(qBound(0.0, colorF, 1.0) * unitValue<quint16>());
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
                                      0 + std::numeric_limits<float>::epsilon(),
                                      1.0f / 255.0f,
                                      0.1, 0.2, 0.5, 0.8, 0.9,
                                      1.0 - 1.0f / 255.0f,
                                      1.0 - std::numeric_limits<float>::epsilon(),
                                      1.0});
    //std::vector<qreal> opacityValues({1.0});
    std::vector<qreal> alphaValues({1.0});
    //std::vector<qreal> alphaValues = opacityValues;
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


                        float tolerance = opacity > 0.999 ? 0.001 :
                                          opacity > 0.5 ? 10 :
                                          opacity > 0.2 ? 100 :
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


                        if (!flags.testFlag(HDR)) {
                            KoColor resultColorU2 = dstColorU;

                            opU2->composite(resultColorU2.data(), 0, srcColorU.data(), 0,
                                            0, 0,
                                            1, 1,
                                            opacity);

                            const quint16 referenceColor = getColorValueU(resultColorU2);
                            const quint16 resultColor = getColorValueU(resultColorU);

                            if (resultColor != referenceColor) {

                                qDebug() << "--- integer implementation is inconsistent to the original mode! ---";
                                qDebug() << ppVar(opacity);
                                qDebug() << "U16 result:   " << fixed << qSetRealNumberPrecision(8)
                                         << "s:" << dumpPixel(srcColorU) << "+" << "d:" << dumpPixel(dstColorU) << "->" << dumpPixel(resultColorU);
                                qDebug() << "U16 reference:" << fixed << qSetRealNumberPrecision(8)
                                         << "s:" << dumpPixel(srcColorU) << "+" << "d:" << dumpPixel(dstColorU) << "->" << dumpPixel(resultColorU2);
                                QFAIL("integer implementation is inconsistent to the original mode!");
                            }
                        }

                        QVERIFY(!std::isnan(resultColorValueF));
                        QVERIFY(!std::isinf(resultColorValueF));

                        if (dstColorValue >= -std::numeric_limits<float>::epsilon() &&
                                dstColorValue <= 1.0 + std::numeric_limits<float>::epsilon() &&
                                srcColorValue >= -std::numeric_limits<float>::epsilon() &&
                                srcColorValue <= 1.0 + std::numeric_limits<float>::epsilon() &&
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

                        if (flags.testFlag(SrcCannotMakeNegative) &&
                                dstColorValue >= 0.0f &&
                                dstColorValue <= 1.0f &&
                                resultColorValueF < 0.0) {

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

                            qDebug() << "--- resulting value in outside SDR range for clipped op! ---";
                            qDebug() << ppVar(opacity);
                            qDebug() << "U16:" << fixed << qSetRealNumberPrecision(8)
                                     << "s:" << dumpPixel(srcColorU) << "+" << "d:" << dumpPixel(dstColorU) << "->" << dumpPixel(resultColorU);
                            qDebug() << "F32:" << fixed << qSetRealNumberPrecision(8)
                                     << "s:" << dumpPixel(srcColorF) << "+" << "d:" << dumpPixel(dstColorF) << "->" << dumpPixel(resultColorF);
                            QFAIL("resulting value in outside SDR range for clipped op!");
                        }

#if 0
                        if (flags.testFlag(HDR)) {
                            // just verify sanity of the result

                            if (dstColorValue >= 0.0 && resultColorValueF < 0.0) {

                                qDebug() << "--- resulting value is negative! ---";
                                qDebug() << "U16:" << fixed << qSetRealNumberPrecision(8)
                                         << "s:" << dumpPixel(srcColorU) << "+" << "d:" << dumpPixel(dstColorU) << "->" << dumpPixel(resultColorU);
                                qDebug() << "F32:" << fixed << qSetRealNumberPrecision(8)
                                         << "s:" << dumpPixel(srcColorF) << "+" << "d:" << dumpPixel(dstColorF) << "->" << dumpPixel(resultColorF);
                                QFAIL("result color value is negative!");
                            }

                            if (qFuzzyIsNull(resultColorValueU) &&
                                    dstColorValue > std::numeric_limits<float>::epsilon() &&
                                    dstColorValue < 1.0 - std::numeric_limits<float>::epsilon()) {

                                if (!qFuzzyIsNull(resultColorValueF)) {

                                    qDebug() << "--- null point position is inconsistent! ---";
                                    qDebug() << "U16:" << fixed << qSetRealNumberPrecision(8)
                                             << "s:" << dumpPixel(srcColorU) << "+" << "d:" << dumpPixel(dstColorU) << "->" << dumpPixel(resultColorU);
                                    qDebug() << "F32:" << fixed << qSetRealNumberPrecision(8)
                                             << "s:" << dumpPixel(srcColorF) << "+" << "d:" << dumpPixel(dstColorF) << "->" << dumpPixel(resultColorF);
                                    //QFAIL("result null point position is inconsistent!");
                                }
                            }

                        } else if (dstColorValue >= -std::numeric_limits<float>::epsilon() &&
                                   dstColorValue <= 1.0 + std::numeric_limits<float>::epsilon() &&
                                   qAbs(resultColorValueF - resultColorValueU) > 0.001) {

                            if (flags.testFlag(AllowClampedComparisonInSDR) &&
                                    qFuzzyIsNull(resultColorValueU) &&
                                    qAbs(resultColorValueU - qMax(0.0f, resultColorValueF)) <= 0.001) {
                                // noop, should be fine result
                            } else if (flags.testFlag(AllowClampedComparisonInSDR) &&
                                       qFuzzyCompare(resultColorValueU, 1.0f) &&
                                       qAbs(resultColorValueU - qMin(1.0f, resultColorValueF)) <= 0.001) {
                                // noop, should be fine result as well
                            } else if (!flags.testFlag(UndefinedIfSrcInHDRRange) ||
                                       (srcColorValue >= -std::numeric_limits<float>::epsilon() &&
                                        srcColorValue <= 1.0 + std::numeric_limits<float>::epsilon())) {

                                // in non-HDR mode both blending modes should behave in
                                // exactly the same way (including zero and unit values)

                                qDebug() << "--- inconsistent result between int and float_sdr modes ---";
                                qDebug() << "U16:" << fixed << qSetRealNumberPrecision(8)
                                         << "s:" << dumpPixel(srcColorU) << "+" << "d:" << dumpPixel(dstColorU) << "->" << dumpPixel(resultColorU);
                                qDebug() << "F32:" << fixed << qSetRealNumberPrecision(8)
                                         << "s:" << dumpPixel(srcColorF) << "+" << "d:" << dumpPixel(dstColorF) << "->" << dumpPixel(resultColorF);
                                QFAIL("inconsistent result between int and float_sdr modes!");
                            }
                        }
#endif
                    }
                }
            }
        }
    }

//    {
//        using namespace Arithmetic;
//        quint16 x = 52428;
//        qDebug() << ppVar(x) << ppVar(inv(x)) << ppVar(unitValue<quint16>() - x);

//        float y = 0.8;
//        qDebug() << ppVar(y) << ppVar(inv(y)) << ppVar(unitValue<float>() - y);
//        qDebug() << ppVar(div(y, inv(y)));
//        qDebug() << ppVar(div(x, inv(x)));
//        qDebug() << ppVar(clampToSDR<float>(div(y, inv(y))));
//        qDebug() << ppVar(clampToSDR<quint16>(div(x, inv(x))));

//    }
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
//    ids << COMPOSITE_GREATER;

//    ids << COMPOSITE_OVERLAY;
//    ids << COMPOSITE_GRAIN_MERGE;
//    ids << COMPOSITE_GRAIN_EXTRACT;
//    ids << COMPOSITE_HARD_MIX;
//    ids << COMPOSITE_HARD_MIX_PHOTOSHOP;
//    ids << COMPOSITE_HARD_MIX_SOFTER_PHOTOSHOP;
//    ids << COMPOSITE_GEOMETRIC_MEAN;
//    ids << COMPOSITE_PARALLEL;
//    ids << COMPOSITE_ALLANON;
//    ids << COMPOSITE_HARD_OVERLAY;
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
    ids << COMPOSITE_HARD_LIGHT;
//    ids << COMPOSITE_SOFT_LIGHT_IFS_ILLUSIONS;
//    ids << COMPOSITE_SOFT_LIGHT_PEGTOP_DELPHI;
//    ids << COMPOSITE_GAMMA_LIGHT;
//    ids << COMPOSITE_GAMMA_ILLUMINATION;
    ids << COMPOSITE_VIVID_LIGHT;
    ids << COMPOSITE_FLAT_LIGHT;
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
//    ids << COMPOSITE_ARC_TANGENT;
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

//    ids << COMPOSITE_DISSOLVE;

//    ids << COMPOSITE_LUMINOSITY_SAI;

    QStringList specialIds;
    specialIds << COMPOSITE_ADD;
    specialIds << COMPOSITE_SUBTRACT;
    specialIds << COMPOSITE_INVERSE_SUBTRACT;
    specialIds << COMPOSITE_MULT;
    specialIds << COMPOSITE_DIVIDE;

    specialIds << COMPOSITE_EXCLUSION;
    specialIds << COMPOSITE_DIFF;
    specialIds << COMPOSITE_SCREEN;
    specialIds << COMPOSITE_DARKEN;
    specialIds << COMPOSITE_LIGHTEN;
    specialIds << COMPOSITE_NEGATION;


    // ids << COMPOSITE_SOFT_LIGHT_SVG;
    // ids << COMPOSITE_SOFT_LIGHT_PHOTOSHOP;

    QTest::addColumn<QString>("id");
    QTest::addColumn<TestFlags>("flags");

    std::vector<bool> isHDRValues({false, true});

    Q_FOREACH (const QString &id, ids) {
        Q_FOREACH (const bool isHDR, isHDRValues) {
            TestFlags flags = isHDR ? HDR : None;
            flags |= SrcCannotMakeNegative;

            if (!isHDR) {
                //flags |= ForceClippedResult;
            }

            QTest::addRow("%s_%s", id.toLatin1().data(), isHDR ? "hdr" : "sdr") << id << flags;
        }
    }

    Q_FOREACH (const QString &id, specialIds) {
        TestFlags flags = None;
        QTest::addRow("%s_%s", id.toLatin1().data(), "sdr") << id << flags;
    }

    // both channels are clamped, sdr only!
    QTest::addRow("%s_%s", COMPOSITE_SOFT_LIGHT_SVG.toLatin1().data(), "sdr") << COMPOSITE_SOFT_LIGHT_SVG << TestFlags(SrcCannotMakeNegative/* | ForceClippedResult*/);
    QTest::addRow("%s_%s", COMPOSITE_SOFT_LIGHT_PHOTOSHOP.toLatin1().data(), "sdr") << COMPOSITE_SOFT_LIGHT_PHOTOSHOP << TestFlags(SrcCannotMakeNegative/* | ForceClippedResult*/);
//    QTest::addRow("%s_%s", COMPOSITE_DIVIDE.toLatin1().data(), "sdr") << COMPOSITE_DIVIDE << TestFlags(None);

}

SIMPLE_TEST_MAIN(TestCompositeOpInversion)
