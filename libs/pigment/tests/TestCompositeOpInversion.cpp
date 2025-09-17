/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "TestCompositeOpInversion.h"
#include "KoColorModelStandardIds.h"

#include <cstring>

#include <simpletest.h>

#include <QPainter>

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
     * Comparing of CMYK against RGB would be more preferable, but it just
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
     *  All other blendmodes have complex formulas (or alpha-composition),
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
    // const KoColorSpace* csF = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), 0);

    KisColorPairSampler sampler;

    sampler.alphaValues = {0.0, 0.3, 0.9};
    sampler.redColorValues = {0.1, 0.4, 0.6, 1.0};

    KisColorPairSampler::const_iterator it = sampler.begin();
    KisColorPairSampler::const_iterator end = sampler.end();

    QCOMPARE(sampler.numSamples(), 3 * 3 * 3 * 4 * 4);
    QCOMPARE(std::distance(it, end), sampler.numSamples());

    for (; it != end; ++it) {
    //     qDebug() << ppVar(it.opacity()) << ppVar(it.srcColor(csF)) << ppVar(it.dstColor(csF));
    }
}

void TestCompositeOpInversion::testColorPairSamplerRGB()
{
    // const KoColorSpace* csF = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), 0);

    KisColorPairSampler sampler;

    sampler.alphaValues = {0.0, 0.3, 0.9};
    sampler.redColorValues = {0.1, 0.4, 0.6, 1.0};
    sampler.greenColorValues = {0.3, 0.7};
    sampler.blueColorValues = {0.2, 0.8};

    KisColorPairSampler::const_iterator it = sampler.begin();
    KisColorPairSampler::const_iterator end = sampler.end();

    qDebug() << ppVar(sampler.numSamples());

    QCOMPARE(sampler.numSamples(), 3 * 3 * 3 * 4 * 4 * 2 * 2 * 2 * 2);
    QCOMPARE(std::distance(it, end), sampler.numSamples());

    for (; it != end; ++it) {
        //qDebug() << ppVar(it.opacity()) << ppVar(it.srcColor(csF)) << ppVar(it.dstColor(csF));
    }
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

Wrapper dumpPixel(const KoColor &color, int channelIndex = 0) {
    return {color, channelIndex};
}

float getColorValue(const KoColor &c, int channelIndex = 0) {
    float result = 777.0;

    if (c.colorSpace()->colorDepthId() == Float32BitsColorDepthID) {
        const float *ptr = reinterpret_cast<const float*>(c.data());
        result = ptr[channelIndex];
    } else if (c.colorSpace()->colorDepthId() == Float16BitsColorDepthID) {
        const half *ptr = reinterpret_cast<const half*>(c.data());
        result = ptr[channelIndex];
    } else if (c.colorSpace()->colorDepthId() == Integer16BitsColorDepthID) {
        using namespace Arithmetic;
        const quint16 *ptr = reinterpret_cast<const quint16*>(c.data());
        const int realChannelIndex = channelIndex == 3 ? channelIndex : 2 - channelIndex;
        result = qreal(ptr[realChannelIndex]) / unitValue<quint16>();
    }

    return result;
}

void dumpChannelsState(qreal opacity, float tolerance,
                       int numChannelsToTest,
                       int failedChannelIndex,
                       const KoColor &srcColorF,
                       const KoColor &dstColorF,
                       const KoColor &resultColorF,
                       const KoColor &srcColorU,
                       const KoColor &dstColorU,
                       const KoColor &resultColorU)
{
    qDebug() << "--- failed to verify the channels ---";
    qDebug() << ppVar(opacity) << ppVar(tolerance);

    if (numChannelsToTest > 1) {
        qDebug();
        qDebug() << ppVar(srcColorF);
        qDebug() << ppVar(dstColorF);
        qDebug() << ppVar(resultColorF);
        qDebug();
        qDebug() << ppVar(srcColorU);
        qDebug() << ppVar(dstColorU);
        qDebug() << ppVar(resultColorU);
        qDebug();

        const QString channelName = srcColorF.colorSpace()->channels()[failedChannelIndex]->name();
        qDebug() << "Failed channel:" << channelName;
    }
    qDebug() << "U16:" << Qt::fixed << qSetRealNumberPrecision(8)
             << "s:" << dumpPixel(srcColorU, failedChannelIndex) << "+" << "d:" << dumpPixel(dstColorU, failedChannelIndex) << "->" << dumpPixel(resultColorU, failedChannelIndex);
    qDebug() << "F32:" << Qt::fixed << qSetRealNumberPrecision(8)
             << "s:" << dumpPixel(srcColorF, failedChannelIndex) << "+" << "d:" << dumpPixel(dstColorF, failedChannelIndex) << "->" << dumpPixel(resultColorF, failedChannelIndex);
}

void dumpChannelsState(qreal opacity, float tolerance,
                       int numChannelsToTest,
                       int failedChannelIndex,
                       const KoColor &srcColorF,
                       const KoColor &dstColorF,
                       const KoColor &resultColorF)
{
    qDebug() << "--- failed to verify the channels ---";
    qDebug() << ppVar(opacity) << ppVar(tolerance);

    if (numChannelsToTest > 1) {
        qDebug();
        qDebug() << ppVar(srcColorF);
        qDebug() << ppVar(dstColorF);
        qDebug() << ppVar(resultColorF);
        qDebug();

        const QString channelName = srcColorF.colorSpace()->channels()[failedChannelIndex]->name();
        qDebug() << "Failed channel:" << channelName;
    }
    qDebug() << "F32:" << Qt::fixed << qSetRealNumberPrecision(8)
             << "s:" << dumpPixel(srcColorF, failedChannelIndex) << "+" << "d:" << dumpPixel(dstColorF, failedChannelIndex) << "->" << dumpPixel(resultColorF, failedChannelIndex);
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
    SampleWholeRGBRange = 0x100,
    SdrRangeCanGenerateSmallErrors = 0x200,
    GenerateF32SampleSheetOnly = 0x400
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
std::vector<qreal> generateNarrowOpacityValues()
{
    return {
            1.0
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

template<typename T = float>
std::vector<qreal> generateNarrowColorValues()
{
    return {-0.1,
            0 - std::numeric_limits<T>::epsilon(),
            0,
            0 + std::numeric_limits<T>::epsilon(),
            0.2,
            0.5 - 2 * std::numeric_limits<T>::epsilon(),
            0.5,
            0.5 + 2 * std::numeric_limits<T>::epsilon(),
            0.8,
            1.0 - std::numeric_limits<T>::epsilon(),
            1.0,
            1.0 + std::numeric_limits<T>::epsilon(),
            1.1};
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

    result.emplace_back(COMPOSITE_TANGENT_NORMALMAP, SampleWholeRGBRange | SdrRangePreserveStable | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_COMBINE_NORMAL, SampleWholeRGBRange | SdrRangePreserveStable | PositivePreserveStable | GenerateF32SampleSheetOnly);

    result.emplace_back(COMPOSITE_COLOR, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_HUE, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_SATURATION, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_INC_SATURATION, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_DEC_SATURATION, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_LUMINIZE, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_INC_LUMINOSITY, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_DEC_LUMINOSITY, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);

    result.emplace_back(COMPOSITE_COLOR_HSI, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_HUE_HSI, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_SATURATION_HSI, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_INC_SATURATION_HSI, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_DEC_SATURATION_HSI, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_INTENSITY, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_INC_INTENSITY, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_DEC_INTENSITY, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);

    result.emplace_back(COMPOSITE_COLOR_HSL, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_HUE_HSL, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_SATURATION_HSL, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_INC_SATURATION_HSL, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_DEC_SATURATION_HSL, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_LIGHTNESS, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_INC_LIGHTNESS, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_DEC_LIGHTNESS, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);

    result.emplace_back(COMPOSITE_COLOR_HSV, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_HUE_HSV, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_SATURATION_HSV, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_INC_SATURATION_HSV, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_DEC_SATURATION_HSV, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_VALUE, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_INC_VALUE, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_DEC_VALUE, SampleWholeRGBRange | SdrRangePreserveStable | SdrRangeCanGenerateSmallErrors | PositivePreserveStable | GenerateF32SampleSheetOnly);

    result.emplace_back(COMPOSITE_DARKER_COLOR, SampleWholeRGBRange | SdrRangePreserveStable | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_LIGHTER_COLOR, SampleWholeRGBRange | SdrRangePreserveStable | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_LAMBERT_LIGHTING, SampleWholeRGBRange | SdrRangePreserveStable | PositivePreserveStable | GenerateF32SampleSheetOnly);
    result.emplace_back(COMPOSITE_LAMBERT_LIGHTING_GAMMA_2_2, SampleWholeRGBRange | SdrRangePreserveStable | PositivePreserveStable | GenerateF32SampleSheetOnly);

    return result;
}

void addAllOps(const std::vector<std::pair<QString, TestFlags>> &ops)
{
    QTest::addColumn<QString>("id");
    QTest::addColumn<TestFlags>("flags");

    auto fixId = [] (QString id) {
        id.replace(' ', '_');
        id.replace('.', '_');
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

int initSampler(KisColorPairSampler &sampler, TestFlags flags)
{
    int numChannelsToTest = 0;

    if (flags.testFlag(SampleWholeRGBRange)) {
        sampler.alphaValues = generateNarrowOpacityValues();
        sampler.redColorValues = generateNarrowColorValues();
        sampler.greenColorValues = generateNarrowColorValues();
        sampler.blueColorValues = generateNarrowColorValues();
        numChannelsToTest = 3;
    } else {
        sampler.alphaValues = generateOpacityValues();
        sampler.redColorValues = generateWideColorValues();
        numChannelsToTest = 1;
    }

    return numChannelsToTest;
}

void TestCompositeOpInversion::testF32ModesNaN()
{
    QFETCH(QString, id);
    QFETCH(TestFlags, flags);

    const KoColorSpace* csF = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), 0);
    const KoCompositeOp *opF = createOp(csF, id, flags.testFlag(HDR));

    KisColorPairSampler sampler;
    const int numChannelsToTest = initSampler(sampler, flags);

    for (auto it = sampler.begin(); it != sampler.end(); ++it) {
        KoColor srcColorF = it.srcColor(csF);
        KoColor dstColorF = it.dstColor(csF);
        KoColor resultColorF = dstColorF;

        opF->composite(resultColorF.data(), 0, srcColorF.data(), 0,
                       0, 0,
                       1, 1,
                       it.opacity());

        for (int channelIndex = 0; channelIndex < numChannelsToTest; ++channelIndex) {
            float resultColorValueF = getColorValue(resultColorF, channelIndex);

            if (std::isnan(resultColorValueF)) {
                dumpChannelsState(it.opacity(), 0,
                                  numChannelsToTest,
                                  channelIndex,
                                  srcColorF,
                                  dstColorF,
                                  resultColorF);
                QFAIL("NaN value is found!");
            }

            if (std::isinf(resultColorValueF)) {
                dumpChannelsState(it.opacity(), 0,
                                  numChannelsToTest,
                                  channelIndex,
                                  srcColorF,
                                  dstColorF,
                                  resultColorF);
                QFAIL("inf value is found!");
            }
        }
    }
}
#if 0
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
    sampler.redColorValues = generateWideColorValues();

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

        if (id == COMPOSITE_COMBINE_NORMAL && Arithmetic::isZeroValueStrict(it.srcColor())) {
            continue;
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
            qDebug() << "U16 result:   " << Qt::fixed << qSetRealNumberPrecision(8)
                     << "s:" << dumpPixel(srcColorU) << "+" << "d:" << dumpPixel(dstColorU) << "->" << dumpPixel(resultColorU);
            qDebug() << "U16 reference:" << Qt::fixed << qSetRealNumberPrecision(8)
                     << "s:" << dumpPixel(srcColorU) << "+" << "d:" << dumpPixel(dstColorU) << "->" << dumpPixel(refResultColorU);
            QFAIL("integer implementation is inconsistent to the original mode!");
        }
    }
}
#endif
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
    const int numChannelsToTest = initSampler(sampler, flags);

    // allocate the temporary buffers **outside** the main loop
    QVector<float> srcColorsF(4);
    QVector<float> dstColorsF(4);
    QVector<float> resultColorsF(4);
    QVector<float> resultColorsU(4);

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

        KoColor srcColorU = it.srcColor(csU);
        KoColor dstColorU = it.dstColor(csU);
        KoColor resultColorU = dstColorU;

        opU->composite(resultColorU.data(), 0, srcColorU.data(), 0,
                       0, 0,
                       1, 1,
                       it.opacity());

        csF->normalisedChannelsValue(srcColorF.data(), srcColorsF);
        csF->normalisedChannelsValue(dstColorF.data(), dstColorsF);
        csF->normalisedChannelsValue(resultColorF.data(), resultColorsF);
        csU->normalisedChannelsValue(resultColorU.data(), resultColorsU);
        std::swap(resultColorsU[0], resultColorsU[2]);

        for (int channelIndex = 0; channelIndex < numChannelsToTest; channelIndex++) {
            const float srcColorValueF = srcColorsF[channelIndex];
            const float dstColorValueF = dstColorsF[channelIndex];

            const float resultColorValueF = resultColorsF[channelIndex];
            const float resultColorValueU = resultColorsU[channelIndex];

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
                checkInSdrRange(dstColorValueF) &&
                checkInSdrRange(srcColorValueF);

            if (isInSdrRange &&
                qAbs(resultColorValueF - resultColorValueU) > tolerance) {

                /**
                 * HARD_MIX_PHOTOSHOP is not contiguous near the unit sum point,
                 * so we should just skip this test case
                 */
                if (id == COMPOSITE_HARD_MIX_PHOTOSHOP &&
                    Arithmetic::isUnitValueFuzzy<float>(float(srcColorValueF) + float(dstColorValueF))) {
                    continue;
                }

                if (id == COMPOSITE_HARD_OVERLAY) {
                    if (Arithmetic::isUnitValueClampedFuzzy<float>(srcColorValueF) &&
                        Arithmetic::isZeroValueClampedFuzzy<float>(dstColorValueF)) {

                        continue;
                    }
                }

                if (id == COMPOSITE_HARD_OVERLAY && flags.testFlag(HDR)) {
                    if (Arithmetic::isUnitValueClampedFuzzy<float>(srcColorValueF)) {
                        continue;
                    }
                }

                if (id == COMPOSITE_DODGE) {
                    if (Arithmetic::isUnitValueClampedFuzzy<float>(srcColorValueF) &&
                        Arithmetic::isZeroValueClampedFuzzy<float>(dstColorValueF)) {
                        continue;
                    }
                }

                if (id == COMPOSITE_DODGE && flags.testFlag(HDR)) {
                    if (Arithmetic::isUnitValueClampedFuzzy<float>(srcColorValueF)) {
                        continue;
                    }
                }

                if (id == COMPOSITE_BURN) {
                    if (Arithmetic::isZeroValueClampedFuzzy<float>(srcColorValueF) &&
                        Arithmetic::isUnitValueClampedFuzzy<float>(dstColorValueF)) {
                        continue;
                    }
                }

                if (id == COMPOSITE_VIVID_LIGHT) {
                    if (Arithmetic::isUnitValueClampedFuzzy<float>(srcColorValueF) ||
                        Arithmetic::isZeroValueClampedFuzzy<float>(srcColorValueF)) {
                        continue;
                    }
                }

                if (id == COMPOSITE_DIVIDE) {
                    if (Arithmetic::isZeroValueClampedFuzzy<float>(srcColorValueF) &&
                        Arithmetic::isZeroValueClampedFuzzy<float>(dstColorValueF)) {
                        continue;
                    }
                }

                if (id == COMPOSITE_ARC_TANGENT) {
                    if (Arithmetic::isZeroValueClampedFuzzy<float>(srcColorValueF) &&
                        Arithmetic::isZeroValueClampedFuzzy<float>(dstColorValueF)) {
                        continue;
                    }
                }

                if (id == COMPOSITE_GAMMA_DARK) {
                    if (Arithmetic::isZeroValueClampedFuzzy<float>(srcColorValueF) &&
                        Arithmetic::isUnitValueClampedFuzzy<float>(dstColorValueF)) {
                        continue;
                    }
                }

                if (id == COMPOSITE_GAMMA_LIGHT) {
                    if (Arithmetic::isZeroValueClampedFuzzy<float>(dstColorValueF)) {
                        continue;
                    }
                }

                if (id == COMPOSITE_GAMMA_ILLUMINATION) {
                    if (Arithmetic::isUnitValueClampedFuzzy<float>(srcColorValueF) &&
                        Arithmetic::isZeroValueClampedFuzzy<float>(dstColorValueF)) {
                        continue;
                    }
                }

                if (id == COMPOSITE_COMBINE_NORMAL) {
                    if (Arithmetic::isHalfValueFuzzy<float>(srcColorValueF) ||
                        Arithmetic::isHalfValueFuzzy<float>(dstColorValueF)) {
                        continue;
                    }
                }

                // all blendmodes that have setSaturation(dst) are unstable near the point
                // when all three channels have the same value
                if (id == COMPOSITE_SATURATION
                    || id == COMPOSITE_SATURATION_HSV
                    || id == COMPOSITE_SATURATION_HSI
                    || id == COMPOSITE_SATURATION_HSL
                    || id == COMPOSITE_DEC_SATURATION
                    || id == COMPOSITE_DEC_SATURATION_HSV
                    || id == COMPOSITE_DEC_SATURATION_HSI
                    || id == COMPOSITE_DEC_SATURATION_HSL
                    || id == COMPOSITE_INC_SATURATION
                    || id == COMPOSITE_INC_SATURATION_HSV
                    || id == COMPOSITE_INC_SATURATION_HSI
                    || id == COMPOSITE_INC_SATURATION_HSL
                    || id == COMPOSITE_HUE
                    || id == COMPOSITE_HUE_HSV
                    || id == COMPOSITE_HUE_HSI
                    || id == COMPOSITE_HUE_HSL) {
                    if (qFuzzyCompare(dstColorsF[0], dstColorsF[1]) &&
                        qFuzzyCompare(dstColorsF[1], dstColorsF[2])) {
                        continue;
                    }
                }

                // all hue related modes cannot handle the case when the source is gray
                if (id == COMPOSITE_HUE
                    || id == COMPOSITE_HUE_HSV
                    || id == COMPOSITE_HUE_HSI
                    || id == COMPOSITE_HUE_HSL) {
                    if (qFuzzyCompare(srcColorsF[0], srcColorsF[1]) &&
                        qFuzzyCompare(srcColorsF[1], srcColorsF[2])) {
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
                    dumpChannelsState(it.opacity(), tolerance,
                                      numChannelsToTest,
                                      channelIndex,
                                      srcColorF, dstColorF, resultColorF,
                                      srcColorU, dstColorU, resultColorU);
                    QFAIL("resulting value in SDR range is negative!");
                } else {
                    dumpChannelsState(it.opacity(), tolerance,
                                      numChannelsToTest,
                                      channelIndex,
                                      srcColorF, dstColorF, resultColorF,
                                      srcColorU, dstColorU, resultColorU);
                    QFAIL("inconsistent result in SDR range!");
                }
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
    const int numChannelsToTest = initSampler(sampler, flags);

    auto checkInSdrRangeImpl = [=] (qreal value, bool useStrictRange) {
        return useStrictRange ?
            (value >= 0.0 && value <= 1.0) :
            (value >= -std::numeric_limits<float>::epsilon() &&
             value <= 1.0 + std::numeric_limits<float>::epsilon());
    };

    auto checkInSdrRange = [=] (qreal value) {
        return checkInSdrRangeImpl(value, useStrictRange);
    };

    QVector<float> srcColorsF(4);
    QVector<float> dstColorsF(4);
    QVector<float> resultColorsF(4);

    for (auto it = sampler.begin(); it != sampler.end(); ++it) {
        KoColor srcColorF = it.srcColor(csF);
        KoColor dstColorF = it.dstColor(csF);
        KoColor resultColorF = dstColorF;

        opF->composite(resultColorF.data(), 0, srcColorF.data(), 0,
                       0, 0,
                       1, 1,
                       it.opacity());

        csF->normalisedChannelsValue(srcColorF.data(), srcColorsF);
        csF->normalisedChannelsValue(dstColorF.data(), dstColorsF);
        csF->normalisedChannelsValue(resultColorF.data(), resultColorsF);

        for (int channelIndex = 0; channelIndex < numChannelsToTest; channelIndex++) {
            const float srcColorValueF = srcColorsF[channelIndex];
            const float dstColorValueF = dstColorsF[channelIndex];
            float resultColorValueF = resultColorsF[channelIndex];

            if ((checkInSdrRange(srcColorValueF)) &&
                checkInSdrRange(dstColorValueF) &&
                !checkInSdrRangeImpl(resultColorValueF, !flags.testFlag(SdrRangeCanGenerateSmallErrors) && useStrictRange)) {

                bool resultConvergedToAPoint = false;

                std::vector<float> transitionalValues;
                transitionalValues.reserve(20 + 1);

                for (int i = 0; i < 20; i++) {

                    transitionalValues.push_back(resultColorValueF);

                    opF->composite(resultColorF.data(), 0, srcColorF.data(), 0,
                                   0, 0,
                                   1, 1,
                                   it.opacity());
                    resultColorValueF = getColorValue(resultColorF, channelIndex);

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
                    qFuzzyCompare(float(srcColorValueF), 1.0f) &&
                    qFuzzyCompare(float(srcColorValueF), 1.0f)) {

                    skipConvergencyCheck = true;
                }

                if (id == COMPOSITE_GREATER) {
                    skipConvergencyCheck = true;
                }

                if (checkInSdrRangeImpl(transitionalValues.back(), true) ||
                    ((resultConvergedToAPoint ||
                      // finalError < originalError ||
                      skipConvergencyCheck) &&
                     qAbs(transitionalValues.back() - transitionalValues.front())
                         < 64 * std::numeric_limits<float>::epsilon())) {

                           // noop, everything is fine
                } else {
                    for (size_t i = 0; i < transitionalValues.size(); i++) {
                        qDebug() << Qt::fixed << qSetRealNumberPrecision(14)
                                 << "    " << i << ":" << transitionalValues[i];
                    }

                    dumpChannelsState(it.opacity(), useStrictRange ? 0.0f : std::numeric_limits<float>::epsilon(),
                                      numChannelsToTest, channelIndex,
                                      srcColorF, dstColorF, resultColorF);
                    QFAIL("op does not preserve SDR range!");
                }
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
                                    return op.second.testFlag(SrcCannotMakeNegative) &&
                                        // we don't test RGB blendmodes for this
                                        !op.second.testFlag(SampleWholeRGBRange);
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
    sampler.redColorValues = generateWideColorValues();

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
            qDebug() << "F32:" << Qt::fixed << qSetRealNumberPrecision(8)
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
    const int numChannelsToTest = initSampler(sampler, flags);

    QVector<float> srcColorsF(4);
    QVector<float> dstColorsF(4);
    QVector<float> resultColorsF(4);

    for (auto it = sampler.begin(); it != sampler.end(); ++it) {
        KoColor srcColorF = it.srcColor(csF);
        KoColor dstColorF = it.dstColor(csF);
        KoColor resultColorF = dstColorF;

        opF->composite(resultColorF.data(), 0, srcColorF.data(), 0,
                       0, 0,
                       1, 1,
                       it.opacity());

        csF->normalisedChannelsValue(srcColorF.data(), srcColorsF);
        csF->normalisedChannelsValue(dstColorF.data(), dstColorsF);
        csF->normalisedChannelsValue(resultColorF.data(), resultColorsF);

        for (int channelIndex = 0; channelIndex < numChannelsToTest; channelIndex++) {
            const float srcColorValueF = srcColorsF[channelIndex];
            const float dstColorValueF = dstColorsF[channelIndex];

            const float resultColorValueF = resultColorsF[channelIndex];

            const bool srcCheckValid = useStrictZeroCheck ?
                srcColorValueF > 0.0 : float(srcColorValueF) >= -std::numeric_limits<float>::epsilon();

            const bool dstCheckValid = useStrictZeroCheck ?
                dstColorValueF > 0.0 : float(dstColorValueF) >= -std::numeric_limits<float>::epsilon();

            const bool resultCheckValid = useStrictZeroCheck ?
                resultColorValueF > -1.0f * std::numeric_limits<float>::epsilon() :
                resultColorValueF >= -2.0f * std::numeric_limits<float>::epsilon();

            if (srcCheckValid &&
                dstCheckValid &&
                !resultCheckValid) {

                dumpChannelsState(it.opacity(), 0.0f,
                                  numChannelsToTest,
                                  channelIndex,
                                  srcColorF, dstColorF, resultColorF);

                QFAIL("resulting value in SDR range generates negative result!");
            }
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
    samplerF16.redColorValues = generateWideColorValues<half>();

    KisColorPairSampler samplerF32;
    samplerF32.alphaValues = generateOpacityValues<float>();
    samplerF32.redColorValues = generateWideColorValues<float>();


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
            qDebug() << "F32:" << Qt::fixed << qSetRealNumberPrecision(8)
                     << "s:" << dumpPixel(srcColorF32) << "+" << "d:" << dumpPixel(dstColorF32) << "->" << dumpPixel(resultColorF32);
            qDebug() << "F16:" << Qt::fixed << qSetRealNumberPrecision(8)
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

void TestCompositeOpInversion::generateSampleSheetsLong_data()
{
    auto ids = generateCompositeOpIdSet();

    QTest::addColumn<QString>("id");
    QTest::addColumn<TestFlags>("flags");
    QTest::addColumn<KoID>("depthId");

    auto fixId = [] (QString id) {
        id.replace(' ', '_');
        return id;
    };

    for (auto it = ids.begin(); it != ids.end(); ++it) {
        const QString id = it->first;
        const TestFlags flags = it->second;
        const bool isHDR = flags.testFlag(HDR);

        std::vector<KoID> depthIds;

        if (flags.testFlag(GenerateF32SampleSheetOnly)) {
            depthIds.push_back(Float32BitsColorDepthID);
        } else {
            depthIds.push_back(Integer16BitsColorDepthID);
            depthIds.push_back(Float32BitsColorDepthID);
            depthIds.push_back(Float16BitsColorDepthID);
        }

        for (auto depthIt = depthIds.begin(); depthIt != depthIds.end(); ++depthIt) {

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
                    KoColor dst = createColor(csF32, dstGradient(qreal(y) / (patchHeight - 1)));
                    KoColor src = createColor(csF32, srcGradient(qreal(x) / (patchWidth - 1)));

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
    //auto g2w = [&] (qreal i) { return makeQRgba64F(i,   1.0, i,   1.0);};
    auto b2w = [&] (qreal i) { return makeQRgba64F(i,   i,   1.0, 1.0);};

    auto r2k = [&] (qreal i) { return makeQRgba64F(1.0 - i, 0,       0,       1.0);};
    //auto g2k = [&] (qreal i) { return makeQRgba64F(0,       1.0 - i, 0,       1.0);};
    auto b2k = [&] (qreal i) { return makeQRgba64F(0,       0,       1.0 - i, 1.0);};

    auto r2a = [&] (qreal i) { return makeQRgba64F(1.0, 0,   0,   1.0 - i);};
    //auto g2a = [&] (qreal i) { return makeQRgba64F(0,   1.0, 0,   1.0 - i);};
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

void TestCompositeOpInversion::testToneMappingPositive()
{
    auto testToneMapping = [&] (const QLatin1String &name, auto HSXType) {
        float r = 0.92727380990982;
        float g = 0.95727380990982;
        float b = 1.15454604625702;

        qDebug();
        qDebug() << "src:" << ppVar(r) << ppVar(g) << ppVar(b);

        ToneMapping<decltype(HSXType), float>(r, g, b);

        QVERIFY(r <= g);
        QVERIFY(g <= b);

        qDebug() << name << ppVar(r) << ppVar(g) << ppVar(b);
    };


    testToneMapping(QLatin1String("HSV"), HSVType{});
    testToneMapping(QLatin1String("HSL"), HSLType{});
    testToneMapping(QLatin1String("HSI"), HSIType{});
    testToneMapping(QLatin1String("HSY"), HSYType{});
}

void TestCompositeOpInversion::testToneMappingNegative()
{
    auto testToneMapping = [&] (const QLatin1String &name, auto HSXType) {
        float r = 0.12727380990982;
        float g = 0.12727380990982;
        float b = -0.15454604625702;

        qDebug();
        qDebug() << "src:" << ppVar(r) << ppVar(g) << ppVar(b);

        ToneMapping<decltype(HSXType), float>(r, g, b);

        QVERIFY(r >= g);
        QVERIFY(g >= b);

        qDebug() << name << ppVar(r) << ppVar(g) << ppVar(b);
    };


    testToneMapping(QLatin1String("HSV"), HSVType{});
    testToneMapping(QLatin1String("HSL"), HSLType{});
    testToneMapping(QLatin1String("HSI"), HSIType{});
    testToneMapping(QLatin1String("HSY"), HSYType{});
}

SIMPLE_TEST_MAIN(TestCompositeOpInversion)
