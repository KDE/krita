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


}

const KoCompositeOp* createOp(const KoColorSpace *cs, const QString &id, bool isHDR);

enum TestFlag {
    None = 0x0,
    HDR = 0x1,
    SrcCannotMakeNegative = 0x2,
    UseStrictSdrRange = 0x4,
    PreservesSdrRange = 0x8,
    PreservesStrictSdrRange = 0x10,
    HasUnboundedRange = 0x20
};
Q_DECLARE_FLAGS(TestFlags, TestFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(TestFlags)
Q_DECLARE_METATYPE(TestFlags)

std::vector<qreal> generateOpacityValues()
{
    return {
        1.0,
        1.0 - 2 * std::numeric_limits<float>::epsilon(),
        1.0 - 1.0f / 255.0f,
        0.1, 0.2, 0.5, 0.8, 0.9,
        1.0f / 255.0f,
        0 + 2 * std::numeric_limits<float>::epsilon(),
        0.0
        };
}

std::vector<qreal> generateWideColorValues()
{
    return {-0.1,
            0 - std::numeric_limits<float>::epsilon(),
            0,
            0 + std::numeric_limits<float>::epsilon(),
            0.1, 0.2,
            0.5 - 2 * std::numeric_limits<float>::epsilon(),
            0.5,
            0.5 + 2 * std::numeric_limits<float>::epsilon(),
            0.8, 0.9,
            1.0 - std::numeric_limits<float>::epsilon(),
            1.0,
            1.0 + std::numeric_limits<float>::epsilon(),
            1.1,
            1.5};
}

std::vector<std::pair<QString, TestFlags>> generateCompositeOpIdSet()
{
    std::vector<std::pair<QString, TestFlags>> result;

    auto addSdrPreservingHdrOp = [&] (const QString &id) {
        result.emplace_back(id, HDR | SrcCannotMakeNegative | PreservesStrictSdrRange);
        result.emplace_back(id, SrcCannotMakeNegative | PreservesSdrRange);
    };

    addSdrPreservingHdrOp(COMPOSITE_PIN_LIGHT);
    addSdrPreservingHdrOp(COMPOSITE_BURN);
    addSdrPreservingHdrOp(COMPOSITE_LINEAR_BURN);

    auto addSdrNonPreservingHdrOp = [&] (const QString &id) {
        result.emplace_back(id, HDR | SrcCannotMakeNegative);
        result.emplace_back(id, SrcCannotMakeNegative | PreservesSdrRange);
    };

    addSdrNonPreservingHdrOp(COMPOSITE_HARD_MIX);
    addSdrNonPreservingHdrOp(COMPOSITE_HARD_OVERLAY);
    addSdrNonPreservingHdrOp(COMPOSITE_DODGE);
    addSdrNonPreservingHdrOp(COMPOSITE_VIVID_LIGHT);

    auto addSdrPreservingOp = [&] (const QString &id) {
        result.emplace_back(id, PreservesSdrRange);
    };

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

    auto addStrictSdrPreservingOp = [&] (const QString &id) {
        result.emplace_back(id, PreservesStrictSdrRange);
    };

    addStrictSdrPreservingOp(COMPOSITE_MULT);
    addStrictSdrPreservingOp(COMPOSITE_DIFF);
    addStrictSdrPreservingOp(COMPOSITE_SCREEN);


    auto addUnboundedRangeOp = [&] (const QString &id) {
        result.emplace_back(id, HasUnboundedRange);
    };

    addUnboundedRangeOp(COMPOSITE_ADD);
    addUnboundedRangeOp(COMPOSITE_SUBTRACT);
    addUnboundedRangeOp(COMPOSITE_INVERSE_SUBTRACT);
    addUnboundedRangeOp(COMPOSITE_DIVIDE);
    addUnboundedRangeOp(COMPOSITE_LUMINOSITY_SAI);
    addUnboundedRangeOp(COMPOSITE_EXCLUSION);
    addUnboundedRangeOp(COMPOSITE_NEGATION);
    addUnboundedRangeOp(COMPOSITE_GRAIN_MERGE);
    addUnboundedRangeOp(COMPOSITE_GRAIN_EXTRACT);

    // both channels are clamped, sdr only!
    result.emplace_back(COMPOSITE_SOFT_LIGHT_SVG, SrcCannotMakeNegative | PreservesSdrRange);
    result.emplace_back(COMPOSITE_SOFT_LIGHT_PHOTOSHOP, SrcCannotMakeNegative | PreservesSdrRange);

    result.emplace_back(COMPOSITE_HARD_MIX_PHOTOSHOP, SrcCannotMakeNegative | UseStrictSdrRange | PreservesStrictSdrRange);
    result.emplace_back(COMPOSITE_HARD_MIX_SOFTER_PHOTOSHOP, SrcCannotMakeNegative | UseStrictSdrRange | PreservesStrictSdrRange);
    result.emplace_back(COMPOSITE_ARC_TANGENT, UseStrictSdrRange | PreservesStrictSdrRange);

    // doesn't clamp result, always preserves SDR state
    result.emplace_back(COMPOSITE_HARD_LIGHT, SrcCannotMakeNegative | PreservesStrictSdrRange);
    result.emplace_back(COMPOSITE_PARALLEL, SrcCannotMakeNegative | PreservesSdrRange);
    result.emplace_back(COMPOSITE_EQUIVALENCE, SrcCannotMakeNegative | PreservesStrictSdrRange);
    result.emplace_back(COMPOSITE_GEOMETRIC_MEAN, SrcCannotMakeNegative | PreservesSdrRange);
    result.emplace_back(COMPOSITE_ADDITIVE_SUBTRACTIVE, SrcCannotMakeNegative | PreservesSdrRange);
    result.emplace_back(COMPOSITE_GAMMA_DARK, SrcCannotMakeNegative | PreservesStrictSdrRange);
    result.emplace_back(COMPOSITE_GAMMA_LIGHT, SrcCannotMakeNegative | PreservesSdrRange);
    result.emplace_back(COMPOSITE_GAMMA_ILLUMINATION, SrcCannotMakeNegative | PreservesStrictSdrRange);

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
        if (Arithmetic::isZeroValue<float>(minOpacity)) continue;

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
            return checkInSdrRangeImpl(value, flags.testFlag(UseStrictSdrRange));
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
                Arithmetic::isUnitValue<float>(float(it.srcColor()) + float(it.dstColor()))) {
                continue;
            }

            if (Arithmetic::isZeroValue<float>(resultColorValueU) &&
                qAbs(resultColorValueU - qMax(0.0f, resultColorValueF)) <= 0.001) {
                // noop, should be fine result
            } else if (Arithmetic::isUnitValue<float>(resultColorValueU) &&
                       qAbs(resultColorValueU - qMin(1.0f, resultColorValueF)) <= 0.001) {
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
                                    return op.second.testFlag(PreservesSdrRange) ||
                                        op.second.testFlag(PreservesStrictSdrRange);
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
                                    return op.second.testFlag(PreservesSdrRange);
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



SIMPLE_TEST_MAIN(TestCompositeOpInversion)
