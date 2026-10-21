/*
 * SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "KoGamutMappingUtils.h"
#include <kis_global.h>

#include <KoColorModelStandardIds.h>
#include <KoColorSpace.h>
#include <KoColor.h>
#include <KoColorConversions.h>
#include <KoColorTransferFunctions.h>

#include <QtMath>

float KoGamutMappingUtils::deltaE76(const QVector<float> &a, const QVector<float> &b) {
    //This is E76, E2000 implementation is too complicated and buggy.
    float dL = a[0] - b[0];
    float dA = a[1] - b[1];
    float dB = a[2] - b[2];
    return qSqrt(dL * dL + dA * dA + dB * dB);
}

float KoGamutMappingUtils::reinhardToneMapping(const float linearY, const float maxY)
{
    const float num = linearY * (1.0f + (linearY/(maxY*maxY)));
    return num / (linearY + 1.0f);
}

//---------- Piecewise Filmic Curve -----------//

static void SolveAB(float & lnA, float & B, float x0, float y0, float m)
{
    B = (m*x0)/y0;
    lnA = logf(y0) - B*logf(x0);
}

static void AsSlopeIntercept(float & m, float & b, float x0, float x1, float y0, float y1)
{
    float dy = (y1-y0);
    float dx = (x1-x0);
    if (dx == 0)
        m = 1.0f;
    else
        m = dy/dx;

    b = y0 - x0*m;
}

static float EvalDerivativeLinearGamma(float m, float b, float g, float x)
{
    float ret = g*m*powf(m*x+b,g-1.0f);
    return ret;
}

KoGamutMappingUtils::PiecewiseFilmicCurve::PiecewiseFilmicCurve(float maxWhite, QPointF point0, QPointF point1, QPointF overshootPoint)
{
    segments = QVector<Segment>(3);

    p0 = point0;
    p1 = point1;
    QPointF overshoot = overshootPoint;

    inverseMaxWhite = 1.0/maxWhite;
    // normalize



    p0.setX(p0.x()/maxWhite);
    p1.setX(p1.x()/maxWhite);

    overshoot.setX(overshootPoint.x()/maxWhite);

    Segment linear;
    float m, b;
    AsSlopeIntercept(m, b, p0.x(), p1.x(), p0.y(), p1.y());
    const float g = 1.0;

    linear.offsetX = -(b / m);
    linear.lnA = g * logf(m);
    linear.b = g;

    segments[1] = linear;

    const float toeM = EvalDerivativeLinearGamma(m, b, g, p0.x());
    const float shoulderM = EvalDerivativeLinearGamma(m, b, g, p1.x());

    Segment toe;
    SolveAB(toe.lnA, toe.b, p0.x(), p0.y(), toeM);
    segments[0] = toe;

    Segment shoulder;
    const float x0 = (1.0f + overshoot.x()) - p1.x();
    const float y0 = (1.0f + overshoot.y()) - p1.y();
    SolveAB(shoulder.lnA, shoulder.b, x0, y0, shoulderM);

    shoulder.offsetX = (1.0f + overshoot.x());
    shoulder.offsetY = (1.0f + overshoot.y());
    shoulder.scaleX = -1.0;
    shoulder.scaleY = -1.0;

    segments[2] = shoulder;

    {
        const float scale = shoulder.evalSegment(1.0f);
        const float invScale = 1.0f / scale;

        segments[0].offsetY *= invScale;
        segments[0].scaleY *= invScale;

        segments[1].offsetY *= invScale;
        segments[1].scaleY *= invScale;

        segments[2].offsetY *= invScale;
        segments[2].scaleY *= invScale;
    }
}

float KoGamutMappingUtils::PiecewiseFilmicCurve::evalCurve(const float x)
{
    int index = (x < p0.x()) ? 0 : ((x < p1.x()) ? 1 : 2);
    Segment s = segments.at(index);
    return s.evalSegment(x*inverseMaxWhite);
}

float KoGamutMappingUtils::PiecewiseFilmicCurve::Segment::evalSegment(const float x)
{
    float x0 = (x - offsetX) * scaleX;
    float y0 = x0 > 0? expf(lnA + b * logf(x0)): 0.0;
    return y0 * scaleY + offsetY;
}

KoGamutMappingUtils::PiecewiseFilmicCurve KoGamutMappingUtils::PiecewiseFilmicCurve::fromUserParams(float toeStrength, float toeLength, float shoulderStrength, float shoulderLength, float shoulderAngle)
{
    const float uiGamma = 2.2f;

    const float normalizedToeLength = powf(qBound(0.0f, toeLength, 1.0f), uiGamma);
    const float normalizedToeStrength = qBound(0.0f, toeStrength, 1.0f);
    const float normalizedShoulderAngle = qBound(0.0f, shoulderAngle, 1.0f);

    const float x0 = normalizedToeLength * .5f;
    const float y0 = (1.0f - normalizedToeStrength) * x0;

    const float remainingY = 1.0f - y0;
    const float initialW = x0 + remainingY;

    const float y1_offset = (1.0f - shoulderStrength) * remainingY;
    const float x1 = x0 + y1_offset;
    const float y1 = y0 + y1_offset;

    const float extraW = exp2f(shoulderLength)-1.0f;
    const float W = initialW + extraW;

    QPointF point0(powf(x0, uiGamma), powf(y0, uiGamma));
    QPointF point1(powf(x1, uiGamma), powf(y1, uiGamma));
    float maxWhite = W;

    QPointF overshoot;
    overshoot.setX((W * 2.0f) * normalizedShoulderAngle * shoulderStrength);
    overshoot.setY(0.5f * normalizedShoulderAngle * shoulderStrength);

    return PiecewiseFilmicCurve(maxWhite, point0, point1, overshoot);
}

const QMatrix4x4 xyz50To65 {QMatrix4x4(
    0.955473421488075,    -0.02309845494876471,   0.06325924320057072, 0.0,
    -0.0283697093338637,    1.0099953980813041,    0.021041441191917323, 0.0,
    0.012314014864481998, -0.020507649298898964,  1.330365926242124, 0.0,
    0.0, 0.0, 0.0, 1.0
    )};

const QMatrix4x4 xyz50To65_inv = xyz50To65.inverted();

const QMatrix4x4 XYZtoOkLABLMS {QMatrix4x4(
    0.8190224379967030, 0.3619062600528904, -0.1288737815209879, 0.0,
    0.0329836539323885, 0.9292868615863434,  0.0361446663506424, 0.0,
    0.0481771893596242, 0.2642395317527308,  0.6335478284694309, 0.0,
    0.0, 0.0, 0.0, 1.0
    )};

const QMatrix4x4 XYZtoOkLABLMS_inv = XYZtoOkLABLMS.inverted();

const QMatrix4x4 lmsToOkLab {QMatrix4x4(
    0.2104542683093140,  0.7936177747023054, -0.0040720430116193, 0.0,
    1.9779985324311684, -2.4285922420485799,  0.4505937096174110, 0.0,
    0.0259040424655478,  0.7827717124575296, -0.8086757549230774, 0.0,
    0.0, 0.0, 0.0, 1.0
    )};

const QMatrix4x4 lmsToOkLab_inv = lmsToOkLab.inverted();

const QMatrix4x4 XYZd65toDolbyLMS {QMatrix4x4(
    0.3592832590121217,  0.6976051147779502, -0.0358915932320290, 0.0,
    -0.1920808463704993,  1.1004767970374321,  0.0753748658519118, 0.0,
    0.0070797844607479,  0.0748396662186362,  0.8433265453898765, 0.0,
    0.0, 0.0, 0.0, 1.0
    )};

const QMatrix4x4 XYZd65toDolbyLMS_inv = XYZd65toDolbyLMS.inverted();

const QMatrix4x4 lmsToICtCp {QMatrix4x4(
    2048.0 / 4096.0,   2048.0 / 4096.0,       0.0 , 0.0,
    6610.0 / 4096.0, -13613.0 / 4096.0,  7003.0 / 4096.0 , 0.0,
    17933.0 / 4096.0, -17390.0 / 4096.0,  -543.0 / 4096.0, 0.0,
    0.0, 0.0, 0.0, 1.0
    )};

const QMatrix4x4 lmsToICtCp_inv = lmsToICtCp.inverted();

QVector4D KoGamutMappingUtils::XYZToOkLab(const QVector4D &xyz) {
    const QVector4D lms = XYZtoOkLABLMS * (xyz50To65 * xyz);

    const QVector4D lmsPrime = QVector4D(cbrtf(lms.x()),
                                         cbrtf(lms.y()),
                                         cbrtf(lms.z()), 0.0);

    return lmsToOkLab * lmsPrime;
}

QVector4D KoGamutMappingUtils::OkLabToXYZ(const QVector4D &okLab) {
    const QVector4D lmsPrime = lmsToOkLab_inv * okLab;
    const QVector4D lms = QVector4D(powf(lmsPrime.x(), 3.0),
                                    powf(lmsPrime.y(), 3.0),
                                    powf(lmsPrime.z(), 3.0), 0.0);
    return xyz50To65_inv * (XYZtoOkLABLMS_inv * lms);
}

QVector4D KoGamutMappingUtils::XYZToICtCp(const QVector4D &xyz)
{
    const QVector4D lms =  XYZd65toDolbyLMS * (xyz50To65 * xyz);

    const QVector4D lmsPrime = QVector4D(applySmpte2048Curve(lms.x()),
                                         applySmpte2048Curve(lms.y()),
                                         applySmpte2048Curve(lms.z()), 0.0);

    return lmsToICtCp * lmsPrime;
}

QVector4D KoGamutMappingUtils::ICtCpToXYZ(const QVector4D &iCtCp)
{
    const QVector4D lmsPrime = lmsToICtCp_inv * iCtCp;
    const QVector4D lms = QVector4D(removeSmpte2048Curve(lmsPrime.x()),
                                    removeSmpte2048Curve(lmsPrime.y()),
                                    removeSmpte2048Curve(lmsPrime.z()), 0.0);
    return xyz50To65_inv * (XYZd65toDolbyLMS_inv * lms);
}

QMap<float, QPointF> KoGamutMappingUtils::getGamutCuspDescription(const KoColorProfile *profile, KoGamutMappingUtils::MappingModel model, const int samplesPerChannel)
{
    QMap<float, QPointF> bounds;
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->colorSpace(profile->colorModelID(), Integer8BitsColorDepthID.id(), profile);
    const KoColorSpace *xyzCs = KoColorSpaceRegistry::instance()->colorSpace(XYZAColorModelID.id(), Float32BitsColorDepthID.id(), "");
    const KoColorSpace *labCs = KoColorSpaceRegistry::instance()->colorSpace(LABAColorModelID.id(), Float32BitsColorDepthID.id(), "");
    QVector<float> xyzChannels(xyzCs->channelCount());
    QVector<float> labChannels(xyzCs->channelCount());

    QVector<QVector<float>> colorList;
    if (profile->colorModelID() == RGBAColorModelID.id()) {
        for (int i = 0; i < samplesPerChannel; i++) {
            const float ch = float(i)/float(samplesPerChannel-1);
            QVector<float> c = {ch, 1.0, 0.0, 1.0};
            colorList << c;
            QVector<float> c2 = {ch, 0.0, 1.0, 1.0};
            colorList << c2;
        }
        for (int i = 0; i < samplesPerChannel; i++) {
            const float ch = float(i)/float(samplesPerChannel-1);
            QVector<float> c = {1.0, ch, 0.0, 1.0};
            colorList << c;
            QVector<float> c2 = {0.0, ch, 1.0, 1.0};
            colorList << c2;
        }
        for (int i = 0; i < samplesPerChannel; i++) {
            const float ch = float(i)/float(samplesPerChannel-1);
            QVector<float> c = {1.0, 0.0, ch, 1.0};
            colorList << c;
            QVector<float> c2 = {0.0, 1.0, ch, 1.0};
            colorList << c2;
        }
    } else if (profile->colorModelID() == CMYKAColorModelID.id()) {
        for (int i = 0; i < samplesPerChannel; i++) {
            const float ch = float(i)/float(samplesPerChannel-1);
            QVector<float> c = {ch, 1.0, 0.0, 0.0, 1.0};
            colorList << c;
            QVector<float> c2 = {ch, 0.0, 1.0, 0.0, 1.0};
            colorList << c2;
        }
        for (int i = 0; i < samplesPerChannel; i++) {
            const float ch = float(i)/float(samplesPerChannel-1);
            QVector<float> c = {1.0, ch, 0.0, 0.0, 1.0};
            colorList << c;
            QVector<float> c2 = {0.0, ch, 1.0, 0.0, 1.0};
            colorList << c2;
        }
        for (int i = 0; i < samplesPerChannel; i++) {
            const float ch = float(i)/float(samplesPerChannel-1);
            QVector<float> c = {1.0, 0.0, ch, 0.0, 1.0};
            colorList << c;
            QVector<float> c2 = {0.0, 1.0, ch, 0.0, 1.0};
            colorList << c2;
        }
    }
    KoColor c(cs);

    Q_FOREACH( const QVector<float> channels, colorList) {
        cs->fromNormalisedChannelsValue(c.data(), channels);
        std::array<qreal, 3> lch;
        if (model == LAB) {
            KoColor labC;
            cs->convertPixelsTo(c.data(), labC.data(), labCs,  1,
                                KoColorConversionTransformation::IntentRelativeColorimetric,
                                KoColorConversionTransformation::adjustmentConversionFlags());
            labCs->normalisedChannelsValue(labC.data(), labChannels);
            LabToLCH(labChannels[0], labChannels[1], labChannels[2], &lch[0], &lch[1], &lch[2]);
        } else {

            KoColor xyzC;
            cs->convertPixelsTo(c.data(), xyzC.data(), xyzCs,  1,
                                KoColorConversionTransformation::IntentRelativeColorimetric,
                                KoColorConversionTransformation::adjustmentConversionFlags());
            xyzCs->normalisedChannelsValue(xyzC.data(), xyzChannels);

            if (model == ICtCp) {
                QVector4D ictcp = KoGamutMappingUtils::XYZToICtCp(QVector4D (xyzChannels[0], xyzChannels[1], xyzChannels[2], 0));
                LabToLCH(ictcp.x(), ictcp.y()+0.5, ictcp.z()+0.5, &lch[0], &lch[1], &lch[2]);
            } else {
                QVector4D okLab = KoGamutMappingUtils::XYZToOkLab(QVector4D (xyzChannels[0], xyzChannels[1], xyzChannels[2], 0));
                LabToLCH(okLab.x(), okLab.y()+0.5, okLab.z()+0.5, &lch[0], &lch[1], &lch[2]);
            }
        }

        bounds.insert(lch[2], QPointF(lch[1], lch[0]));
    }

    return bounds;
}

QPointF KoGamutMappingUtils::cuspForHue(const QMap<float, QPointF> &gamutCuspDescription, const float hue)
{
    if (gamutCuspDescription.isEmpty()) return QPointF();

    float firstAngle = 0.0;
    float secondAngle = 0.0;

    auto it = gamutCuspDescription.upperBound(hue);
    if (it == gamutCuspDescription.begin()) {
        firstAngle = gamutCuspDescription.lastKey();
        secondAngle = gamutCuspDescription.firstKey();
    } else if (it == gamutCuspDescription.end()) {
        firstAngle = gamutCuspDescription.lastKey();
        secondAngle = gamutCuspDescription.firstKey();
    } else {
        firstAngle = std::prev(it).key();
        secondAngle = it.key();
    }

    const float angle = firstAngle;
    const float nextAngle = secondAngle;

    QPointF lch1 = gamutCuspDescription.value(angle);
    QPointF lch2 = gamutCuspDescription.value(nextAngle);
    const float t = hue < angle
        ? (hue - angle + 1.0) / (nextAngle + 1.0 - angle)
        : (hue - angle) / (nextAngle - angle);
    return QLineF(lch1, lch2).pointAt(t);
}


