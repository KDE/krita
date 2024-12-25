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

#include <KisColorPairSampler.h>

#include "kis_debug.h"

#include <krita_container_utils.h>

namespace KritaUtils
{
template <typename C, typename T>
bool containerContains(const C &container, T &&value) {
    return std::find(container.begin(),
                     container.end(),
                     std::forward<T>(value)) != container.end();

}
}


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

void TestCompositeOpInversion::testColorPairSampler()
{
    const KoColorSpace* csU = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Integer16BitsColorDepthID.id(), 0);
    const KoColorSpace* csF = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), 0);

    KisColorPairSampler sampler;

    sampler.alphaValues = {0.0, 0.3, 0.9};
    sampler.colorValues = {0.1, 0.4, 0.6, 1.0};

    KisColorPairSampler::const_iterator it = sampler.begin();
    KisColorPairSampler::const_iterator end = sampler.end();

    QCOMPARE(sampler.numSamples(), 3 * 3 * 3 * 4 * 4);
    QCOMPARE(std::distance(it, end), sampler.numSamples());

    // for (; it != end; ++it) {
    //     qDebug() << ppVar(it.opacity()) << ppVar(it.srcColor(csF)) << ppVar(it.dstColor(csF));
    // }
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
    } else if (c.colorSpace()->colorDepthId() == Float16BitsColorDepthID) {
        const half *ptr = reinterpret_cast<const half*>(c.data());
        debug.nospace() << "(" << ptr[w.index] << ", " << ptr[3] << ")";
    } else if (c.colorSpace()->colorDepthId() == Integer16BitsColorDepthID) {
        using namespace Arithmetic;
        const quint16 *ptr = reinterpret_cast<const quint16*>(c.data());
        debug.nospace() << "(" << qreal(ptr[2 - w.index]) / unitValue<quint16>() << ", " << qreal(ptr[3]) / unitValue<quint16>() << ")";
    } else {
        qFatal("not implemented");
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
    } else if (c.colorSpace()->colorDepthId() == Float16BitsColorDepthID) {
        const half *ptr = reinterpret_cast<const half*>(c.data());
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

}

const KoCompositeOp* createOp(const KoColorSpace *cs, const QString &id, bool isHDR);

enum TestFlag {
    None = 0x0,
    HDR = 0x1,
    SrcCannotMakeNegative = 0x2,
    VerifyConsistencyInStrictSdrRange = 0x4,
    SdrRangePreserveStable = 0x8,
    SdrRangePreserveUnstable = 0x10,
    HasUnboundedRange = 0x20,
    PositivePreserveStable = 0x40,
    PositivePreserveUnstable = 0x80,
};
Q_DECLARE_FLAGS(TestFlags, TestFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(TestFlags)
Q_DECLARE_METATYPE(TestFlags)

template<typename T = float>
std::vector<qreal> generateOpacityValues()
{
    return {
        1.0,
        1.0 - 2 * std::numeric_limits<T>::epsilon(),
        1.0 - 1.0f / 255.0f,
        0.1, 0.2, 0.5, 0.8, 0.9,
        1.0f / 255.0f,
        0 + 2 * std::numeric_limits<T>::epsilon(),
        0.0
        };
}

template<typename T = float>
std::vector<qreal> generateWideColorValues()
{
    return {-0.1,
            0 - std::numeric_limits<T>::epsilon(),
            0,
            0 + std::numeric_limits<T>::epsilon(),
            0.1, 0.2,
            0.5 - 2 * std::numeric_limits<T>::epsilon(),
            0.5,
            0.5 + 2 * std::numeric_limits<T>::epsilon(),
            0.8, 0.9,
            1.0 - std::numeric_limits<T>::epsilon(),
            1.0,
            1.0 + std::numeric_limits<T>::epsilon(),
            1.1,
            1.5};
}

std::vector<std::pair<QString, TestFlags>> generateCompositeOpIdSet()
{
    std::vector<std::pair<QString, TestFlags>> result;

    auto addSdrPreservingHdrOp = [&] (const QString &id) {
        result.emplace_back(id, HDR | SrcCannotMakeNegative | SdrRangePreserveUnstable | PositivePreserveStable);
        result.emplace_back(id, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    };

    addSdrPreservingHdrOp(COMPOSITE_LINEAR_BURN);

    // Hard-mix is the only stable positive-preserving mode
    result.emplace_back(COMPOSITE_HARD_MIX, HDR | SrcCannotMakeNegative | PositivePreserveStable);
    result.emplace_back(COMPOSITE_HARD_MIX, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);

    result.emplace_back(COMPOSITE_HARD_OVERLAY, HDR | SrcCannotMakeNegative | PositivePreserveUnstable);
    result.emplace_back(COMPOSITE_HARD_OVERLAY, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);

    result.emplace_back(COMPOSITE_DODGE, HDR | SrcCannotMakeNegative | PositivePreserveUnstable);
    result.emplace_back(COMPOSITE_DODGE, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);

    result.emplace_back(COMPOSITE_VIVID_LIGHT, HDR | SrcCannotMakeNegative | PositivePreserveUnstable);
    result.emplace_back(COMPOSITE_VIVID_LIGHT, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);

    auto addSdrPreservingOp = [&] (const QString &id) {
        result.emplace_back(id, SdrRangePreserveStable | PositivePreserveStable);
    };

    addSdrPreservingOp(COMPOSITE_PIN_LIGHT);
    addSdrPreservingOp(COMPOSITE_BURN);
    addSdrPreservingOp(COMPOSITE_DARKEN);
    addSdrPreservingOp(COMPOSITE_LIGHTEN);
    addSdrPreservingOp(COMPOSITE_ALLANON);
    addSdrPreservingOp(COMPOSITE_DISSOLVE);
    addSdrPreservingOp(COMPOSITE_OVER);
    addSdrPreservingOp(COMPOSITE_ALPHA_DARKEN);
    addSdrPreservingOp(COMPOSITE_DESTINATION_IN);
    addSdrPreservingOp(COMPOSITE_DESTINATION_ATOP);
    addSdrPreservingOp(COMPOSITE_ERASE);
    addSdrPreservingOp(COMPOSITE_COPY);
    addSdrPreservingOp(COMPOSITE_GREATER);
    addSdrPreservingOp(COMPOSITE_GRAIN_MERGE);
    addSdrPreservingOp(COMPOSITE_GRAIN_EXTRACT);
    addSdrPreservingOp(COMPOSITE_EXCLUSION);
    addSdrPreservingOp(COMPOSITE_NEGATION);

    auto addStrictSdrPreservingOp = [&] (const QString &id) {
        result.emplace_back(id, SdrRangePreserveUnstable | PositivePreserveStable);
    };

    addStrictSdrPreservingOp(COMPOSITE_MULT);
    addStrictSdrPreservingOp(COMPOSITE_DIFF);
    addStrictSdrPreservingOp(COMPOSITE_SCREEN);


    auto addUnboundedRangeOp = [&] (const QString &id) {
        result.emplace_back(id, HasUnboundedRange | PositivePreserveStable);
    };

    addUnboundedRangeOp(COMPOSITE_ADD);
    addUnboundedRangeOp(COMPOSITE_LUMINOSITY_SAI);




    result.emplace_back(COMPOSITE_DIVIDE, PositivePreserveUnstable);
    result.emplace_back(COMPOSITE_INVERSE_SUBTRACT, HasUnboundedRange);
    result.emplace_back(COMPOSITE_SUBTRACT, HasUnboundedRange);

    // both channels are clamped, sdr only!
    result.emplace_back(COMPOSITE_SOFT_LIGHT_SVG, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_SOFT_LIGHT_PHOTOSHOP, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);

    result.emplace_back(COMPOSITE_HARD_MIX_PHOTOSHOP, SrcCannotMakeNegative | VerifyConsistencyInStrictSdrRange | SdrRangePreserveUnstable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_HARD_MIX_SOFTER_PHOTOSHOP, SrcCannotMakeNegative | VerifyConsistencyInStrictSdrRange | SdrRangePreserveUnstable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_ARC_TANGENT, VerifyConsistencyInStrictSdrRange | SdrRangePreserveUnstable | PositivePreserveUnstable);

    // doesn't clamp result, always preserves SDR state
    result.emplace_back(COMPOSITE_HARD_LIGHT, SrcCannotMakeNegative | SdrRangePreserveUnstable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_PARALLEL, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_EQUIVALENCE, SrcCannotMakeNegative | SdrRangePreserveUnstable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_GEOMETRIC_MEAN, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_ADDITIVE_SUBTRACTIVE, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_GAMMA_DARK, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_GAMMA_LIGHT, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_GAMMA_ILLUMINATION, SrcCannotMakeNegative | SdrRangePreserveUnstable | PositivePreserveStable);

    result.emplace_back(COMPOSITE_TANGENT_NORMALMAP, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_COMBINE_NORMAL, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_COLOR, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_HUE, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_SATURATION, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_INC_SATURATION, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_DEC_SATURATION, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_LUMINIZE, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_INC_LUMINOSITY, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_DEC_LUMINOSITY, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_DARKER_COLOR, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_LIGHTER_COLOR, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_LAMBERT_LIGHTING, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_LAMBERT_LIGHTING_GAMMA_2_2, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_LAMBERT_LIGHTING_GAMMA_2_2, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_COLOR_HSI, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_HUE_HSI, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_SATURATION_HSI, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_INC_SATURATION_HSI, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_DEC_SATURATION_HSI, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_INTENSITY, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_INC_INTENSITY, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_DEC_INTENSITY, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_COLOR_HSL, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_HUE_HSL, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_SATURATION_HSL, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_INC_SATURATION_HSL, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_DEC_SATURATION_HSL, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_LIGHTNESS, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_INC_LIGHTNESS, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_DEC_LIGHTNESS, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_COLOR_HSV, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_HUE_HSV, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_SATURATION_HSV, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_INC_SATURATION_HSV, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_DEC_SATURATION_HSV, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_VALUE, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_INC_VALUE, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);
    result.emplace_back(COMPOSITE_DEC_VALUE, SrcCannotMakeNegative | SdrRangePreserveStable | PositivePreserveStable);

    return result;
}

void addAllOps(const std::vector<std::pair<QString, TestFlags>> &ops)
{
    QTest::addColumn<QString>("id");
    QTest::addColumn<TestFlags>("flags");

    auto fixId = [] (QString id) {
        id.replace(' ', '_');
        return id;
    };

    for (auto it = ops.begin(); it != ops.end(); ++it) {
        const QString id = it->first;
        const TestFlags flags = it->second;
        const bool isHDR = flags.testFlag(HDR);

        QTest::addRow("%s_%s", fixId(id).toLatin1().data(), isHDR ? "hdr" : "sdr") << id << flags;
    }
}

void TestCompositeOpInversion::testF32ModesNaN_data()
{
    addAllOps(generateCompositeOpIdSet());
}

void TestCompositeOpInversion::testF32ModesNaN()
{
    QFETCH(QString, id);
    QFETCH(TestFlags, flags);

    const KoColorSpace* csF = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), 0);
    const KoCompositeOp *opF = createOp(csF, id, flags.testFlag(HDR));

    KisColorPairSampler sampler;
    sampler.alphaValues = generateOpacityValues();
    sampler.colorValues = generateWideColorValues();

    for (auto it = sampler.begin(); it != sampler.end(); ++it) {
        KoColor srcColorF = it.srcColor(csF);
        KoColor dstColorF = it.dstColor(csF);
        KoColor resultColorF = dstColorF;

        opF->composite(resultColorF.data(), 0, srcColorF.data(), 0,
                       0, 0,
                       1, 1,
                       it.opacity());

        float resultColorValueF = getColorValue(resultColorF);

        if (std::isnan(resultColorValueF)) {
            qDebug() << "--- NaN value is found! ---";
            qDebug() << ppVar(it.opacity());
            qDebug() << "F32:" << fixed << qSetRealNumberPrecision(8)
                     << "s:" << dumpPixel(srcColorF) << "+" << "d:" << dumpPixel(dstColorF) << "->" << dumpPixel(resultColorF);
            QFAIL("NaN value is found!");
        }

        if (std::isinf(resultColorValueF)) {
            qDebug() << "--- inf value is found! ---";
            qDebug() << ppVar(it.opacity());
            qDebug() << "F32:" << fixed << qSetRealNumberPrecision(8)
                     << "s:" << dumpPixel(srcColorF) << "+" << "d:" << dumpPixel(dstColorF) << "->" << dumpPixel(resultColorF);
            QFAIL("inf value is found!");
        }
    }
}

void TestCompositeOpInversion::testU16ModesConsistent_data()
{
    auto ids = generateCompositeOpIdSet();

    std::vector<QString> skipCompositeOps;
    skipCompositeOps.push_back(COMPOSITE_DISSOLVE);
    skipCompositeOps.push_back(COMPOSITE_GREATER);
    skipCompositeOps.push_back(COMPOSITE_OVER);
    skipCompositeOps.push_back(COMPOSITE_ALPHA_DARKEN);
    skipCompositeOps.push_back(COMPOSITE_DESTINATION_ATOP);
    skipCompositeOps.push_back(COMPOSITE_DESTINATION_IN);
    skipCompositeOps.push_back(COMPOSITE_ERASE);
    skipCompositeOps.push_back(COMPOSITE_COPY);

    KritaUtils::filterContainer(ids,
        [&] (const std::pair<QString, TestFlags> &op) {
            return !op.second.testFlag(HDR) &&
                !KritaUtils::containerContains(skipCompositeOps, op.first);
    });

    addAllOps(ids);
}

void TestCompositeOpInversion::testU16ModesConsistent()
{
    QFETCH(QString, id);
    QFETCH(TestFlags, flags);

    const KoColorSpace* csU = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Integer16BitsColorDepthID.id(), 0);
    const KoCompositeOp *opU = createOp(csU, id, flags.testFlag(HDR));
    const KoCompositeOp *refOpU = csU->compositeOp(id);

    KisColorPairSampler sampler;
    sampler.alphaValues = generateOpacityValues();
    sampler.colorValues = generateWideColorValues();

    for (auto it = sampler.begin(); it != sampler.end(); ++it) {
        KIS_ASSERT(!flags.testFlag(HDR));
        const qreal minOpacity = std::min({it.opacity(), it.srcAlpha(), it.dstAlpha()});

        // we have added a fast-path for opacity == 0.0 case
        if (Arithmetic::isZeroValueFuzzy<float>(minOpacity)) continue;

        KoColor srcColorU = it.srcColor(csU);
        KoColor dstColorU = it.dstColor(csU);
        KoColor resultColorU = dstColorU;

        opU->composite(resultColorU.data(), 0, srcColorU.data(), 0,
                       0, 0,
                       1, 1,
                       it.opacity());

        KoColor refResultColorU = dstColorU;

        refOpU->composite(refResultColorU.data(), 0, srcColorU.data(), 0,
                          0, 0,
                          1, 1,
                          it.opacity());

        const quint16 referenceColor = getColorValueU(refResultColorU);
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
        if (qFuzzyCompare(float(it.dstAlpha()), 1.0f) ||
            qFuzzyCompare(float(it.srcAlpha()), 1.0f)) {
            maxDifference = 3;
        }

        /**
         * We have added a static point that resolves numerical
         * instability.
         */
        const bool shouldSkipCheck =
            id == COMPOSITE_HARD_LIGHT &&
            (qFuzzyCompare(float(it.srcColor()), 0.5f) ||
             qFuzzyCompare(float(it.dstColor()), 0.5f));

        if (!shouldSkipCheck && difference > maxDifference) {

            qDebug() << "--- integer implementation is inconsistent to the original mode! ---";
            qDebug() << ppVar(it.opacity()) << ppVar(resultColor) << ppVar(referenceColor);
            qDebug() << "U16 result:   " << fixed << qSetRealNumberPrecision(8)
                     << "s:" << dumpPixel(srcColorU) << "+" << "d:" << dumpPixel(dstColorU) << "->" << dumpPixel(resultColorU);
            qDebug() << "U16 reference:" << fixed << qSetRealNumberPrecision(8)
                     << "s:" << dumpPixel(srcColorU) << "+" << "d:" << dumpPixel(dstColorU) << "->" << dumpPixel(refResultColorU);
            QFAIL("integer implementation is inconsistent to the original mode!");
        }
    }
}

void TestCompositeOpInversion::testF32vsU16ConsistencyInSDR_data()
{
    addAllOps(generateCompositeOpIdSet());
}

void TestCompositeOpInversion::testF32vsU16ConsistencyInSDR()
{
    QFETCH(QString, id);
    QFETCH(TestFlags, flags);

    const KoColorSpace* csF = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), 0);
    const KoCompositeOp *opF = createOp(csF, id, flags.testFlag(HDR));

    const KoColorSpace* csU = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Integer16BitsColorDepthID.id(), 0);
    const KoCompositeOp *opU = createOp(csU, id, flags.testFlag(HDR));

    KisColorPairSampler sampler;
    sampler.alphaValues = generateOpacityValues();
    sampler.colorValues = generateWideColorValues();

    for (auto it = sampler.begin(); it != sampler.end(); ++it) {

        const qreal minOpacity = std::min({it.opacity(), it.srcAlpha(), it.dstAlpha()});

        const float tolerance = minOpacity > 0.999 ? 0.001 :
            minOpacity > 0.5 ? 10 :
            minOpacity > 0.2 ? 100 :
            100;

        KoColor srcColorF = it.srcColor(csF);
        KoColor dstColorF = it.dstColor(csF);
        KoColor resultColorF = dstColorF;

        opF->composite(resultColorF.data(), 0, srcColorF.data(), 0,
                       0, 0,
                       1, 1,
                       it.opacity());

        float resultColorValueF = getColorValue(resultColorF);

        KoColor srcColorU = it.srcColor(csU);
        KoColor dstColorU = it.dstColor(csU);
        KoColor resultColorU = dstColorU;

        opU->composite(resultColorU.data(), 0, srcColorU.data(), 0,
                       0, 0,
                       1, 1,
                       it.opacity());

        float resultColorValueU = getColorValue(resultColorU);

        auto checkInSdrRangeImpl = [] (qreal value, bool useStrictRange) {
            return useStrictRange ?
                (value >= 0.0 && value <= 1.0) :
                (value >= -std::numeric_limits<float>::epsilon() &&
                 value <= 1.0 + std::numeric_limits<float>::epsilon());
        };

        auto checkInSdrRange = [=] (qreal value) {
            return checkInSdrRangeImpl(value, flags.testFlag(VerifyConsistencyInStrictSdrRange));
        };

        const bool isInSdrRange =
            checkInSdrRange(it.dstColor()) &&
            checkInSdrRange(it.srcColor());

        if (isInSdrRange &&
            qAbs(resultColorValueF - resultColorValueU) > tolerance) {

            /**
             * HARD_MIX_PHOTOSHOP is not contiguous near the unit sum point,
             * so we should just skip this test case
             */
            if (id == COMPOSITE_HARD_MIX_PHOTOSHOP &&
                Arithmetic::isUnitValueFuzzy<float>(float(it.srcColor()) + float(it.dstColor()))) {
                continue;
            }

            if (id == COMPOSITE_HARD_OVERLAY) {
                if (Arithmetic::isUnitValueClampedFuzzy<float>(it.srcColor()) &&
                    Arithmetic::isZeroValueClampedFuzzy<float>(it.dstColor())) {

                    continue;
                }
            }

            if (id == COMPOSITE_HARD_OVERLAY && flags.testFlag(HDR)) {
                if (Arithmetic::isUnitValueClampedFuzzy<float>(it.srcColor())) {
                    continue;
                }
            }

            if (id == COMPOSITE_DODGE) {
                if (Arithmetic::isUnitValueClampedFuzzy<float>(it.srcColor()) &&
                    Arithmetic::isZeroValueClampedFuzzy<float>(it.dstColor())) {
                    continue;
                }
            }

            if (id == COMPOSITE_DODGE && flags.testFlag(HDR)) {
                if (Arithmetic::isUnitValueClampedFuzzy<float>(it.srcColor())) {
                    continue;
                }
            }

            if (id == COMPOSITE_BURN) {
                if (Arithmetic::isZeroValueClampedFuzzy<float>(it.srcColor()) &&
                    Arithmetic::isUnitValueClampedFuzzy<float>(it.dstColor())) {
                    continue;
                }
            }

            if (id == COMPOSITE_VIVID_LIGHT) {
                if (Arithmetic::isUnitValueClampedFuzzy<float>(it.srcColor()) ||
                    Arithmetic::isZeroValueClampedFuzzy<float>(it.srcColor())) {
                    continue;
                }
            }

            if (id == COMPOSITE_DIVIDE) {
                if (Arithmetic::isZeroValueClampedFuzzy<float>(it.srcColor()) &&
                    Arithmetic::isZeroValueClampedFuzzy<float>(it.dstColor())) {
                    continue;
                }
            }

            if (id == COMPOSITE_ARC_TANGENT) {
                if (Arithmetic::isZeroValueClampedFuzzy<float>(it.srcColor()) &&
                    Arithmetic::isZeroValueClampedFuzzy<float>(it.dstColor())) {
                    continue;
                }
            }

            if (id == COMPOSITE_GAMMA_DARK) {
                if (Arithmetic::isZeroValueClampedFuzzy<float>(it.srcColor()) &&
                    Arithmetic::isUnitValueClampedFuzzy<float>(it.dstColor())) {
                    continue;
                }
            }

            if (id == COMPOSITE_GAMMA_LIGHT) {
                if (Arithmetic::isZeroValueClampedFuzzy<float>(it.dstColor())) {
                    continue;
                }
            }

            if (id == COMPOSITE_GAMMA_ILLUMINATION) {
                if (Arithmetic::isUnitValueClampedFuzzy<float>(it.srcColor()) &&
                    Arithmetic::isZeroValueClampedFuzzy<float>(it.dstColor())) {
                    continue;
                }
            }

            // normal extension into HDR
            bool isStraightUnitExtension =
                Arithmetic::isUnitValueFuzzy<float>(resultColorValueU) &&
                qAbs(resultColorValueU - qMin(1.0f, resultColorValueF)) <= 0.001;

            // clipped extension into HDR (when opacity < 1.0)
            isStraightUnitExtension |=
                resultColorValueU > minOpacity && resultColorValueF > 1.0;

            if (Arithmetic::isZeroValueFuzzy<float>(resultColorValueU) &&
                qAbs(resultColorValueU - qMax(0.0f, resultColorValueF)) <= 0.001) {
                // noop, should be fine result
            } else if (isStraightUnitExtension) {
                // noop, should be fine result as well
            } else if (resultColorValueF < 0.0) {
                qDebug() << "--- resulting value in SDR range is negative! ---";
                qDebug() << ppVar(it.opacity());
                qDebug() << "U16:" << fixed << qSetRealNumberPrecision(8)
                         << "s:" << dumpPixel(srcColorU) << "+" << "d:" << dumpPixel(dstColorU) << "->" << dumpPixel(resultColorU);
                qDebug() << "F32:" << fixed << qSetRealNumberPrecision(8)
                         << "s:" << dumpPixel(srcColorF) << "+" << "d:" << dumpPixel(dstColorF) << "->" << dumpPixel(resultColorF);
                QFAIL("resulting value in SDR range is negative!");
            } else {

                qDebug() << "--- inconsistent result in SDR range ---";
                qDebug() << ppVar(it.opacity());
                qDebug() << "U16:" << fixed << qSetRealNumberPrecision(8)
                         << "s:" << dumpPixel(srcColorU) << "+" << "d:" << dumpPixel(dstColorU) << "->" << dumpPixel(resultColorU);
                qDebug() << "F32:" << fixed << qSetRealNumberPrecision(8)
                         << "s:" << dumpPixel(srcColorF) << "+" << "d:" << dumpPixel(dstColorF) << "->" << dumpPixel(resultColorF);
                QFAIL("inconsistent result in SDR range!");
            }
        }

    }
}

void TestCompositeOpInversion::testPreservesSdrRangeImpl(bool useStrictRange)
{
    QFETCH(QString, id);
    QFETCH(TestFlags, flags);

    const KoColorSpace* csF = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), 0);
    const KoCompositeOp *opF = createOp(csF, id, flags.testFlag(HDR));

    KisColorPairSampler sampler;
    sampler.alphaValues = generateOpacityValues();
    sampler.colorValues = generateWideColorValues();

    auto checkInSdrRangeImpl = [=] (qreal value, bool useStrictRange) {
        return useStrictRange ?
            (value >= 0.0 && value <= 1.0) :
            (value >= -std::numeric_limits<float>::epsilon() &&
             value <= 1.0 + std::numeric_limits<float>::epsilon());
    };

    auto checkInSdrRange = [=] (qreal value) {
        return checkInSdrRangeImpl(value, useStrictRange);
    };

    for (auto it = sampler.begin(); it != sampler.end(); ++it) {
        KoColor srcColorF = it.srcColor(csF);
        KoColor dstColorF = it.dstColor(csF);
        KoColor resultColorF = dstColorF;

        opF->composite(resultColorF.data(), 0, srcColorF.data(), 0,
                       0, 0,
                       1, 1,
                       it.opacity());

        float resultColorValueF = getColorValue(resultColorF);

        if ((checkInSdrRange(it.srcColor())) &&
            checkInSdrRange(it.dstColor()) &&
            !checkInSdrRange(resultColorValueF)) {

            bool resultConvergedToAPoint = false;

            std::vector<float> transitionalValues;
            transitionalValues.reserve(20 + 1);

            for (int i = 0; i < 20; i++) {
                transitionalValues.push_back(resultColorValueF);

                opF->composite(resultColorF.data(), 0, srcColorF.data(), 0,
                               0, 0,
                               1, 1,
                               it.opacity());
                resultColorValueF = getColorValue(resultColorF);

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

            // const float originalError = qAbs(transitionalValues.front() - resultColorValueU);
            // const float finalError = qAbs(transitionalValues.back() - resultColorValueU);

            bool skipConvergencyCheck = false;

            if ((id == COMPOSITE_DARKEN ||
                 id == COMPOSITE_LIGHTEN ||
                 id == COMPOSITE_ALLANON) &&
                qFuzzyCompare(float(it.srcColor()), 1.0f) &&
                qFuzzyCompare(float(it.srcColor()), 1.0f)) {

                skipConvergencyCheck = true;
            }

            if (id == COMPOSITE_GREATER) {
                skipConvergencyCheck = true;
            }

            if (checkInSdrRangeImpl(transitionalValues.back(), true) ||
                ((resultConvergedToAPoint ||
//                  finalError < originalError ||
                  skipConvergencyCheck) &&
                     qAbs(transitionalValues.back() - transitionalValues.front())
                     < 64 * std::numeric_limits<float>::epsilon())) {

                       // noop, everything is fine
            } else {
                qDebug() << "--- op does not preserve SDR range! ---";
                qDebug() << fixed << qSetRealNumberPrecision(8)
                         << ppVar(it.opacity()) << ppVar(it.srcColor()) << ppVar(it.dstColor()) << ppVar(resultColorValueF)
                         << fixed << qSetRealNumberPrecision(14);

                qDebug() << "stabilization track:";
                for (size_t i = 0; i < transitionalValues.size(); i++) {
                    qDebug() << fixed << qSetRealNumberPrecision(14)
                             << "    " << i << ":" << transitionalValues[i];
                }

                qDebug() << "F32:" << fixed << qSetRealNumberPrecision(8)
                         << "s:" << dumpPixel(srcColorF) << "+" << "d:" << dumpPixel(dstColorF) << "->" << dumpPixel(resultColorF);

                QFAIL("op does not preserve SDR range!");
            }
        }
    }
}

void TestCompositeOpInversion::testPreservesStrictSdrRange_data()
{
    auto ids = generateCompositeOpIdSet();

    KritaUtils::filterContainer(ids,
                                [&] (const std::pair<QString, TestFlags> &op) {
                                    return op.second.testFlag(SdrRangePreserveStable) ||
                                        op.second.testFlag(SdrRangePreserveUnstable);
                                });

    addAllOps(ids);
}

void TestCompositeOpInversion::testPreservesStrictSdrRange()
{
    testPreservesSdrRangeImpl(true);
}

void TestCompositeOpInversion::testPreservesLooseSdrRange_data()
{
    auto ids = generateCompositeOpIdSet();

    KritaUtils::filterContainer(ids,
                                [&] (const std::pair<QString, TestFlags> &op) {
                                    return op.second.testFlag(SdrRangePreserveStable);
                                });

    addAllOps(ids);
}

void TestCompositeOpInversion::testPreservesLooseSdrRange()
{
    testPreservesSdrRangeImpl(false);
}

void TestCompositeOpInversion::testSrcCannotMakeNegative_data()
{
    auto ids = generateCompositeOpIdSet();

    KritaUtils::filterContainer(ids,
                                [&] (const std::pair<QString, TestFlags> &op) {
                                    return op.second.testFlag(SrcCannotMakeNegative);
                                });

    addAllOps(ids);
}

void TestCompositeOpInversion::testSrcCannotMakeNegative()
{
    QFETCH(QString, id);
    QFETCH(TestFlags, flags);

    const KoColorSpace* csF = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), 0);
    const KoCompositeOp *opF = createOp(csF, id, flags.testFlag(HDR));

    KisColorPairSampler sampler;
    sampler.alphaValues = generateOpacityValues();
    sampler.colorValues = generateWideColorValues();

    for (auto it = sampler.begin(); it != sampler.end(); ++it) {
        KoColor srcColorF = it.srcColor(csF);
        KoColor dstColorF = it.dstColor(csF);
        KoColor resultColorF = dstColorF;

        opF->composite(resultColorF.data(), 0, srcColorF.data(), 0,
                       0, 0,
                       1, 1,
                       it.opacity());

        float resultColorValueF = getColorValue(resultColorF);

        if (it.dstColor() >= 0.0f &&
            it.dstColor() <= 1.0f &&
            resultColorValueF < -std::numeric_limits<float>::epsilon()) {

            qDebug() << "--- resulting value in SDR range is negative for SRC-clipped op! ---";
            qDebug() << ppVar(it.opacity());
            qDebug() << "F32:" << fixed << qSetRealNumberPrecision(8)
                     << "s:" << dumpPixel(srcColorF) << "+" << "d:" << dumpPixel(dstColorF) << "->" << dumpPixel(resultColorF);
            QFAIL("resulting value in SDR range is negative for SRC-clipped op!");
        }
    }
}

void TestCompositeOpInversion::testPreservesStrictNegative_data()
{
    auto ids = generateCompositeOpIdSet();

    KritaUtils::filterContainer(ids,
                                [&] (const std::pair<QString, TestFlags> &op) {
                                    return op.second.testFlag(PositivePreserveStable) ||
                                        op.second.testFlag(PositivePreserveUnstable);
                                });


    addAllOps(ids);
}

void TestCompositeOpInversion::testPreservesStrictNegative()
{
    testNegativeImpl(true);
}

void TestCompositeOpInversion::testPreservesLooseNegative_data()
{
    auto ids = generateCompositeOpIdSet();

    KritaUtils::filterContainer(ids,
                                [&] (const std::pair<QString, TestFlags> &op) {
                                    return op.second.testFlag(PositivePreserveStable);
                                });


    addAllOps(ids);
}

void TestCompositeOpInversion::testPreservesLooseNegative()
{
    testNegativeImpl(false);
}

void TestCompositeOpInversion::testNegativeImpl(bool useStrictZeroCheck)
{
    QFETCH(QString, id);
    QFETCH(TestFlags, flags);

    const KoColorSpace* csF = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), 0);
    const KoCompositeOp *opF = createOp(csF, id, flags.testFlag(HDR));

    KisColorPairSampler sampler;
    sampler.alphaValues = generateOpacityValues();
    sampler.colorValues = generateWideColorValues();

    for (auto it = sampler.begin(); it != sampler.end(); ++it) {
        KoColor srcColorF = it.srcColor(csF);
        KoColor dstColorF = it.dstColor(csF);
        KoColor resultColorF = dstColorF;

        opF->composite(resultColorF.data(), 0, srcColorF.data(), 0,
                       0, 0,
                       1, 1,
                       it.opacity());

        const float resultColorValueF = getColorValue(resultColorF);

        const bool srcCheckValid = useStrictZeroCheck ?
            it.srcColor() > 0.0 : float(it.srcColor()) >= -std::numeric_limits<float>::epsilon();

        const bool dstCheckValid = useStrictZeroCheck ?
            it.dstColor() > 0.0 : float(it.dstColor()) >= -std::numeric_limits<float>::epsilon();

        const bool resultCheckValid = useStrictZeroCheck ?
            resultColorValueF > -1.0f * std::numeric_limits<float>::epsilon() :
            resultColorValueF >= -2.0f * std::numeric_limits<float>::epsilon();

        if (srcCheckValid &&
            dstCheckValid &&
            !resultCheckValid) {

            qDebug() << "--- resulting value in SDR range generates negative result! ---";
            qDebug() << ppVar(useStrictZeroCheck);
            qDebug() << ppVar(it.opacity());
            qDebug() << "F32:" << fixed << qSetRealNumberPrecision(8)
                     << "s:" << dumpPixel(srcColorF) << "+" << "d:" << dumpPixel(dstColorF) << "->" << dumpPixel(resultColorF);
            QFAIL("resulting value in SDR range generates negative result!");
        }
    }
}



void TestCompositeOpInversion::dumpOpCategories()
{
    auto ids = generateCompositeOpIdSet();
    std::sort(ids.begin(), ids.end());

    auto printCategory = [ids] (const QString &categoryName, TestFlags includeFlags, bool flagsFound = true) {
        qDebug().noquote().nospace() << categoryName << ":";
        for (auto it = ids.begin(); it != ids.end(); ++it) {
            if (bool(it->second & includeFlags) == flagsFound) {
                const QString hdrSuffix = it->second.testFlag(HDR) ? " HDR" : "";

                qDebug().noquote().nospace() << "    * \"" << KoCompositeOpRegistry::instance().getCompositeOpDisplayName(it->first) << "\" (" << it->first << ")" << hdrSuffix;
            }
        }
    };

    qDebug();
    qDebug();
    printCategory("Preserve SDR range (stable)", SdrRangePreserveStable);
    qDebug();
    printCategory("Preserve SDR range (unstable)", SdrRangePreserveUnstable);
    qDebug();
    printCategory("Does NOT preserve SDR range", SdrRangePreserveStable | SdrRangePreserveUnstable, false);

    qDebug();
    qDebug();
    printCategory("Preserve positive range (stable)", PositivePreserveStable);
    qDebug();
    printCategory("Preserve positive range (unstable)", PositivePreserveUnstable);
    qDebug();
    printCategory("Does NOT preserve positive range", PositivePreserveStable | PositivePreserveUnstable, false);
}

void TestCompositeOpInversion::testF16Modes_data()
{
    addAllOps(generateCompositeOpIdSet());
}

void TestCompositeOpInversion::testF16Modes()
{
    QFETCH(QString, id);
    QFETCH(TestFlags, flags);

    const KoColorSpace* csF32 = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), 0);
    const KoCompositeOp *opF32 = createOp(csF32, id, flags.testFlag(HDR));

    const KoColorSpace* csF16 = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float16BitsColorDepthID.id(), 0);
    const KoCompositeOp *opF16 = createOp(csF16, id, flags.testFlag(HDR));

    KisColorPairSampler samplerF16;
    samplerF16.alphaValues = generateOpacityValues<half>();
    samplerF16.colorValues = generateWideColorValues<half>();

    KisColorPairSampler samplerF32;
    samplerF32.alphaValues = generateOpacityValues<float>();
    samplerF32.colorValues = generateWideColorValues<float>();


    for (auto itF16 = samplerF16.begin(), itF32 = samplerF32.begin(); itF16 != samplerF16.end(); ++itF16, ++itF32) {
        KoColor srcColorF32 = itF32.srcColor(csF32);
        KoColor dstColorF32 = itF32.dstColor(csF32);
        KoColor resultColorF32 = dstColorF32;

        opF32->composite(resultColorF32.data(), 0, srcColorF32.data(), 0,
                       0, 0,
                       1, 1,
                       itF32.opacity());

        float resultColorValueF32 = getColorValue(resultColorF32);

        KoColor srcColorF16 = itF16.srcColor(csF16);
        KoColor dstColorF16 = itF16.dstColor(csF16);
        KoColor resultColorF16 = dstColorF16;

        opF16->composite(resultColorF16.data(), 0, srcColorF16.data(), 0,
                         0, 0,
                         1, 1,
                         itF16.opacity());

        float resultColorValueF16 = getColorValue(resultColorF16);

        float tolerance = 4.0f * std::numeric_limits<half>::epsilon();

        if (qAbs(resultColorValueF32 - resultColorValueF16) > tolerance) {

            qDebug() << "--- resulting value in SDR range generates negative result! ---";
            qDebug() << ppVar(itF16.opacity());
            qDebug() << "F32:" << fixed << qSetRealNumberPrecision(8)
                     << "s:" << dumpPixel(srcColorF32) << "+" << "d:" << dumpPixel(dstColorF32) << "->" << dumpPixel(resultColorF32);
            qDebug() << "F16:" << fixed << qSetRealNumberPrecision(8)
                     << "s:" << dumpPixel(srcColorF16) << "+" << "d:" << dumpPixel(dstColorF16) << "->" << dumpPixel(resultColorF16);
            QFAIL("resulting value in SDR range generates negative result!");
        }

    }
}

QString csShortName(const KoID &depthId)
{
    if (depthId == Integer16BitsColorDepthID) {
        return "u16";
    } else if (depthId == Float16BitsColorDepthID) {
        return "f16";
    } else if (depthId == Float32BitsColorDepthID) {
        return "f32";
    } else {
        qFatal("not implemented");
    }
    return "error";
}


void TestCompositeOpInversion::generateSampleSheets_data()
{
    auto ids = generateCompositeOpIdSet();

    QTest::addColumn<QString>("id");
    QTest::addColumn<TestFlags>("flags");
    QTest::addColumn<KoID>("depthId");

    auto fixId = [] (QString id) {
        id.replace(' ', '_');
        return id;
    };

    std::vector<KoID> depthIds = {Integer16BitsColorDepthID, Float32BitsColorDepthID, Float16BitsColorDepthID};


    for (auto it = ids.begin(); it != ids.end(); ++it) {
        for (auto depthIt = depthIds.begin(); depthIt != depthIds.end(); ++depthIt) {
            const QString id = it->first;
            const TestFlags flags = it->second;
            const bool isHDR = flags.testFlag(HDR);

            if (id == COMPOSITE_DISSOLVE) continue;

            if (*depthIt == Integer16BitsColorDepthID && isHDR) continue;

            if (id == COMPOSITE_BURN && !isHDR && *depthIt != Integer16BitsColorDepthID) {
                continue;
            } else if (id == COMPOSITE_LINEAR_BURN && !isHDR && *depthIt != Integer16BitsColorDepthID) {
                continue;
            } else if (id == COMPOSITE_PIN_LIGHT && !isHDR && *depthIt != Integer16BitsColorDepthID) {
                continue;
            }

            QTest::addRow("%s_%s_%s", fixId(id).toLatin1().data(), isHDR ? "hdr" : "sdr", csShortName(*depthIt).toLatin1().data()) << id << flags << *depthIt;
        }
    }
}

#include <QPainter>
void TestCompositeOpInversion::generateSampleSheets()
{
    QFETCH(QString, id);
    QFETCH(TestFlags, flags);
    QFETCH(KoID, depthId);

    const KoColorSpace* csF32 = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), depthId.id(), 0);
    const KoCompositeOp *opF32 = createOp(csF32, id, flags.testFlag(HDR));


    QImage image(QSize(768, 1024), QImage::Format_ARGB32);



    auto createColor = [] (const KoColorSpace *cs, QRgb value) {
        KoColor c(cs);

        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            float *ptr = reinterpret_cast<float*>(c.data());
            ptr[0] = KoColorSpaceMaths<quint8, float>::scaleToA(qRed(value));
            ptr[1] = KoColorSpaceMaths<quint8, float>::scaleToA(qGreen(value));
            ptr[2] = KoColorSpaceMaths<quint8, float>::scaleToA(qBlue(value));
            ptr[3] = KoColorSpaceMaths<quint8, float>::scaleToA(qAlpha(value));
        } else if (cs->colorDepthId() == Float16BitsColorDepthID) {
            half *ptr = reinterpret_cast<half*>(c.data());
            ptr[0] = KoColorSpaceMaths<quint8, half>::scaleToA(qRed(value));
            ptr[1] = KoColorSpaceMaths<quint8, half>::scaleToA(qGreen(value));
            ptr[2] = KoColorSpaceMaths<quint8, half>::scaleToA(qBlue(value));
            ptr[3] = KoColorSpaceMaths<quint8, half>::scaleToA(qAlpha(value));
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            quint16 *ptr = reinterpret_cast<quint16*>(c.data());
            ptr[2] = KoColorSpaceMaths<quint8, quint16>::scaleToA(qRed(value));
            ptr[1] = KoColorSpaceMaths<quint8, quint16>::scaleToA(qGreen(value));
            ptr[0] = KoColorSpaceMaths<quint8, quint16>::scaleToA(qBlue(value));
            ptr[3] = KoColorSpaceMaths<quint8, quint16>::scaleToA(qAlpha(value));
        } else {
            qFatal("bit depth is not implemented");
        }

        return c;
    };

    auto createQRgb = [] (const KoColorSpace *cs, const KoColor &c) {
        QRgb result = 0;

        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            const float *ptr = reinterpret_cast<const float*>(c.data());
            result = qRgba(
                KoColorSpaceMaths<float, quint8>::scaleToA(ptr[0]),
                KoColorSpaceMaths<float, quint8>::scaleToA(ptr[1]),
                KoColorSpaceMaths<float, quint8>::scaleToA(ptr[2]),
                KoColorSpaceMaths<float, quint8>::scaleToA(ptr[3]));
        } else if (cs->colorDepthId() == Float16BitsColorDepthID) {
            const half *ptr = reinterpret_cast<const half*>(c.data());
            result = qRgba(
                KoColorSpaceMaths<half, quint8>::scaleToA(ptr[0]),
                KoColorSpaceMaths<half, quint8>::scaleToA(ptr[1]),
                KoColorSpaceMaths<half, quint8>::scaleToA(ptr[2]),
                KoColorSpaceMaths<half, quint8>::scaleToA(ptr[3]));
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            const quint16 *ptr = reinterpret_cast<const quint16*>(c.data());
            result = qRgba(
                KoColorSpaceMaths<quint16, quint8>::scaleToA(ptr[2]),
                KoColorSpaceMaths<quint16, quint8>::scaleToA(ptr[1]),
                KoColorSpaceMaths<quint16, quint8>::scaleToA(ptr[0]),
                KoColorSpaceMaths<quint16, quint8>::scaleToA(ptr[3]));
        } else {
            qFatal("bit depth is not implemented");
        }

        return result;
    };


    auto drawVerticalGradient =
        [&image, createColor, createQRgb, opF32, csF32] (const QPoint &offset,
                                                        auto dstGradient,
                                                        auto srcGradient,
                                                        const QString &dstText,
                                                        const QString &srcText) {
            for (int y = 0; y < 256; y++) {
                for (int x = 0; x < 256; x++) {
                    KoColor dst = createColor(csF32, dstGradient(x));
                    KoColor src = createColor(csF32, srcGradient(y));

                    opF32->composite(dst.data(), 0, src.data(), 0,
                                     0, 0,
                                     1, 1,
                                     OPACITY_OPAQUE_F);

                    image.setPixel(x + offset.x(), y + offset.y(), createQRgb(csF32, dst));
                }
            }

            QPainter gc(&image);

            QFont font;
            font.setPointSize(10);
            gc.setFont(font);
            gc.setRenderHints(QPainter::TextAntialiasing);
            gc.setPen(QPen(Qt::black, 1));
            gc.drawText(offset + QPoint(100,100) + QPoint(1,1), QString("s: %1").arg(srcText));
            gc.drawText(offset + QPoint(100,115) + QPoint(1,1), QString("d: %1").arg(dstText));
        };

    auto r2w = [] (int i) { return qRgba(255, i,   i,   255);};
    auto g2w = [] (int i) { return qRgba(i,   255, i,   255);};
    auto b2w = [] (int i) { return qRgba(i,   i,   255, 255);};

    auto r2k = [] (int i) { return qRgba(255 - i, 0,       0,       255);};
    auto g2k = [] (int i) { return qRgba(0,       255 - i, 0,       255);};
    auto b2k = [] (int i) { return qRgba(0,       0,       255 - i, 255);};

    auto r2a = [] (int i) { return qRgba(255, 0,   0,   255 - i);};
    auto g2a = [] (int i) { return qRgba(0,   255, 0,   255 - i);};
    auto b2a = [] (int i) { return qRgba(0,   0,   255, 255 - i);};

    auto w2k = [] (int i) { return qRgba(255 - i, 255 - i, 255 - i, 255);};
    auto w2a = [] (int i) { return qRgba(255, 255, 255, 255 - i);};
    auto k2a = [] (int i) { return qRgba(0, 0, 0, 255 - i);};

    auto generateSheet = [&] (const QString &baseName, auto dstGradient) {
        drawVerticalGradient(QPoint(0 * 256, 0 * 256), dstGradient, r2w, baseName, "r2w");
        drawVerticalGradient(QPoint(1 * 256, 0 * 256), dstGradient, r2k, baseName, "r2k");
        drawVerticalGradient(QPoint(2 * 256, 0 * 256), dstGradient, r2a, baseName, "r2a");

        drawVerticalGradient(QPoint(0 * 256, 1 * 256), dstGradient, g2w, baseName, "g2w");
        drawVerticalGradient(QPoint(1 * 256, 1 * 256), dstGradient, g2k, baseName, "g2k");
        drawVerticalGradient(QPoint(2 * 256, 1 * 256), dstGradient, g2a, baseName, "g2a");

        drawVerticalGradient(QPoint(0 * 256, 2 * 256), dstGradient, b2w, baseName, "b2w");
        drawVerticalGradient(QPoint(1 * 256, 2 * 256), dstGradient, b2k, baseName, "b2k");
        drawVerticalGradient(QPoint(2 * 256, 2 * 256), dstGradient, b2a, baseName, "b2a");

        drawVerticalGradient(QPoint(0 * 256, 3 * 256), dstGradient, w2k, baseName, "w2k");
        drawVerticalGradient(QPoint(1 * 256, 3 * 256), dstGradient, w2a, baseName, "w2a");
        drawVerticalGradient(QPoint(2 * 256, 3 * 256), dstGradient, k2a, baseName, "k2a");


        QString hdrString = flags.testFlag(HDR) ? "_hdr" : "_sdr";

        if (id == COMPOSITE_BURN || id == COMPOSITE_LINEAR_BURN || id == COMPOSITE_PIN_LIGHT) {
            hdrString = "";
        }

        image.save(QString("sample_sheet_%1%2_%3_%4.png")
                       .arg(id,
                            hdrString,
                            baseName,
                            csShortName(csF32->colorDepthId())));
    };

    generateSheet("r2w", r2w);
    generateSheet("r2k", r2k);
    generateSheet("w2k", w2k);
    generateSheet("w2a", w2a);
    generateSheet("k2a", k2a);

}

void TestCompositeOpInversion::generateSampleSheetsLong_data()
{
    generateSampleSheets_data();
}

void TestCompositeOpInversion::generateSampleSheetsLong()
{
    QFETCH(QString, id);
    QFETCH(TestFlags, flags);
    QFETCH(KoID, depthId);

    const KoColorSpace* csF32 = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), depthId.id(), 0);
    const KoCompositeOp *opF32 = createOp(csF32, id, flags.testFlag(HDR));


    auto createColor = [] (const KoColorSpace *cs, QRgba64 value) {
        KoColor c(cs);

        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            float *ptr = reinterpret_cast<float*>(c.data());
            ptr[0] = KoColorSpaceMaths<quint16, float>::scaleToA(value.red());
            ptr[1] = KoColorSpaceMaths<quint16, float>::scaleToA(value.green());
            ptr[2] = KoColorSpaceMaths<quint16, float>::scaleToA(value.blue());
            ptr[3] = KoColorSpaceMaths<quint16, float>::scaleToA(value.alpha());
        } else if (cs->colorDepthId() == Float16BitsColorDepthID) {
            half *ptr = reinterpret_cast<half*>(c.data());
            ptr[0] = KoColorSpaceMaths<quint16, half>::scaleToA(value.red());
            ptr[1] = KoColorSpaceMaths<quint16, half>::scaleToA(value.green());
            ptr[2] = KoColorSpaceMaths<quint16, half>::scaleToA(value.blue());
            ptr[3] = KoColorSpaceMaths<quint16, half>::scaleToA(value.alpha());
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            quint16 *ptr = reinterpret_cast<quint16*>(c.data());
            ptr[2] = value.red();
            ptr[1] = value.green();
            ptr[0] = value.blue();
            ptr[3] = value.alpha();
        } else {
            qFatal("bit depth is not implemented");
        }

        return c;
    };

    auto createQRgb = [] (const KoColorSpace *cs, const KoColor &c) -> QRgb {
        QRgba64 result;

        if (cs->colorDepthId() == Float32BitsColorDepthID) {
            const float *ptr = reinterpret_cast<const float*>(c.data());
            result = qRgba64(
                KoColorSpaceMaths<float, quint16>::scaleToA(ptr[0]),
                KoColorSpaceMaths<float, quint16>::scaleToA(ptr[1]),
                KoColorSpaceMaths<float, quint16>::scaleToA(ptr[2]),
                KoColorSpaceMaths<float, quint16>::scaleToA(ptr[3]));
        } else if (cs->colorDepthId() == Float16BitsColorDepthID) {
            const half *ptr = reinterpret_cast<const half*>(c.data());
            result = qRgba64(
                KoColorSpaceMaths<half, quint16>::scaleToA(ptr[0]),
                KoColorSpaceMaths<half, quint16>::scaleToA(ptr[1]),
                KoColorSpaceMaths<half, quint16>::scaleToA(ptr[2]),
                KoColorSpaceMaths<half, quint16>::scaleToA(ptr[3]));
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            const quint16 *ptr = reinterpret_cast<const quint16*>(c.data());
            result = qRgba64(
                KoColorSpaceMaths<quint16, quint16>::scaleToA(ptr[2]),
                KoColorSpaceMaths<quint16, quint16>::scaleToA(ptr[1]),
                KoColorSpaceMaths<quint16, quint16>::scaleToA(ptr[0]),
                KoColorSpaceMaths<quint16, quint16>::scaleToA(ptr[3]));
        } else {
            qFatal("bit depth is not implemented");
        }

        return result.toArgb32();
    };

    const int height = 256;
    const int width = 1536;

    auto drawVerticalGradient =
        [&] (const QPoint &offset,
            auto dstGradient,
            auto srcGradient,
            const QString &dstText,
            const QString &srcText,
            bool flipSrcDst,
            QImage &image) {

            const int &patchWidth = !flipSrcDst ? width : height;
            const int &patchHeight = !flipSrcDst ? height : width;

            for (int y = 0; y < patchHeight; y++) {
                for (int x = 0; x < patchWidth; x++) {
                    KoColor dst = createColor(csF32, dstGradient(qreal(y) / patchHeight));
                    KoColor src = createColor(csF32, srcGradient(qreal(x) / patchWidth));

                    opF32->composite(dst.data(), 0, src.data(), 0,
                                     0, 0,
                                     1, 1,
                                     OPACITY_OPAQUE_F);

                    image.setPixel(x + offset.x(), y + offset.y(), createQRgb(csF32, dst));
                }
            }

            QPainter gc(&image);

            QFont font;
            font.setPointSize(10);
            gc.setFont(font);
            gc.setRenderHints(QPainter::TextAntialiasing);
            gc.setPen(QPen(Qt::black, 1));
            gc.drawText(offset + QPoint(100,100) + QPoint(1,1), QString("s: %1 (x)").arg(srcText));
            gc.drawText(offset + QPoint(100,115) + QPoint(1,1), QString("d: %1 (y)").arg(dstText));
        };

    auto makeQRgba64F = [] (qreal r, qreal g, qreal b, qreal a) {
        return qRgba64(qRound(r * 0xffffu),
                       qRound(g * 0xffffu),
                       qRound(b * 0xffffu),
                       qRound(a * 0xffffu));
    };

    auto r2w = [&] (qreal i) { return makeQRgba64F(1.0, i,   i,   1.0);};
    auto g2w = [&] (qreal i) { return makeQRgba64F(i,   1.0, i,   1.0);};
    auto b2w = [&] (qreal i) { return makeQRgba64F(i,   i,   1.0, 1.0);};

    auto r2k = [&] (qreal i) { return makeQRgba64F(1.0 - i, 0,       0,       1.0);};
    auto g2k = [&] (qreal i) { return makeQRgba64F(0,       1.0 - i, 0,       1.0);};
    auto b2k = [&] (qreal i) { return makeQRgba64F(0,       0,       1.0 - i, 1.0);};

    auto r2a = [&] (qreal i) { return makeQRgba64F(1.0, 0,   0,   1.0 - i);};
    auto g2a = [&] (qreal i) { return makeQRgba64F(0,   1.0, 0,   1.0 - i);};
    auto b2a = [&] (qreal i) { return makeQRgba64F(0,   0,   1.0, 1.0 - i);};

    auto w2k = [&] (qreal i) { return makeQRgba64F(1.0 - i, 1.0 - i, 1.0 - i, 1.0);};
    auto w2a = [&] (qreal i) { return makeQRgba64F(1.0, 1.0, 1.0, 1.0 - i);};
    auto k2a = [&] (qreal i) { return makeQRgba64F(0, 0, 0, 1.0 - i);};

    auto generateSheet = [&] (const QString &baseName, auto dstGradient, bool flipSrcDst) {

        const int stepX = !flipSrcDst ? 0 : height;
        const int stepY = !flipSrcDst ? height : 0;

        const int imageWidth = !flipSrcDst ? width : 9 * height;
        const int imageHeight = !flipSrcDst ? 9 * height : width;

        QImage image(QSize(imageWidth, imageHeight), QImage::Format_ARGB32);

        drawVerticalGradient(QPoint(0 * stepX, 0 * stepY), dstGradient, r2w, baseName, "r2w", flipSrcDst, image);
        drawVerticalGradient(QPoint(1 * stepX, 1 * stepY), dstGradient, r2k, baseName, "r2k", flipSrcDst, image);
        drawVerticalGradient(QPoint(2 * stepX, 2 * stepY), dstGradient, r2a, baseName, "r2a", flipSrcDst, image);

        drawVerticalGradient(QPoint(3 * stepX, 3 * stepY), dstGradient, b2w, baseName, "b2w", flipSrcDst, image);
        drawVerticalGradient(QPoint(4 * stepX, 4 * stepY), dstGradient, b2k, baseName, "b2k", flipSrcDst, image);
        drawVerticalGradient(QPoint(5 * stepX, 5 * stepY), dstGradient, b2a, baseName, "b2a", flipSrcDst, image);

        drawVerticalGradient(QPoint(6 * stepX, 6 * stepY), dstGradient, w2k, baseName, "w2k", flipSrcDst, image);
        drawVerticalGradient(QPoint(7 * stepX, 7 * stepY), dstGradient, w2a, baseName, "w2a", flipSrcDst, image);
        drawVerticalGradient(QPoint(8 * stepX, 8 * stepY), dstGradient, k2a, baseName, "k2a", flipSrcDst, image);


        QString hdrString = flags.testFlag(HDR) ? "_hdr" : "_sdr";

        if (id == COMPOSITE_BURN || id == COMPOSITE_LINEAR_BURN || id == COMPOSITE_PIN_LIGHT) {
            hdrString = "";
        }

        image.save(QString("sample_sheet_%1%2_%3_%4_%5.png")
                       .arg(id,
                            hdrString,
                            baseName,
                            flipSrcDst ? "flip" : "nor",
                            csShortName(csF32->colorDepthId())));
    };

    generateSheet("r2w", r2w, false);
    generateSheet("r2k", r2k, false);
    generateSheet("w2k", w2k, false);
    generateSheet("w2a", w2a, false);
    generateSheet("k2a", k2a, false);

    generateSheet("r2w", r2w, true);
    generateSheet("r2k", r2k, true);
    generateSheet("w2k", w2k, true);
    generateSheet("w2a", w2a, true);
    generateSheet("k2a", k2a, true);

}

void TestCompositeOpInversion::testColor()
{
    const KoColorSpace* csF32 = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), 0);
    const KoCompositeOp *opF32 = createOp(csF32, COMPOSITE_DIVIDE, false);

    KoColor src(QColor(255, 0, 0), csF32);
    KoColor dst(QColor(120, 255, 255), csF32);

    qDebug() << ppVar(src) << ppVar(dst);

    opF32->composite(dst.data(), 0, src.data(), 0,
                     0, 0,
                     1, 1,
                     OPACITY_OPAQUE_F);

    qDebug() << ppVar(dst);


}

SIMPLE_TEST_MAIN(TestCompositeOpInversion)
