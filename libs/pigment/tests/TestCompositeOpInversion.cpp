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

                        op->composite(dstColor.data(), 0, srcColor.data(), 0, 0, 0, 1, 1, opacity);

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

SIMPLE_TEST_MAIN(TestCompositeOpInversion)
