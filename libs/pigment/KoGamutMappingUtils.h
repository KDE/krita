/*
 * SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef KOGAMUTMAPPINGUTILS_H
#define KOGAMUTMAPPINGUTILS_H

#include <QPointF>
#include <QVector>

#include <KoColorimetryUtils.h>
#include <KoColorProfile.h>

#include "kritapigment_export.h"

namespace KoGamutMappingUtils {
/**
 * @brief deltaE76
 * calculates the difference between two colors, represented as normalized channels.
 * @return The difference. Given that the input is scaled to [0 1], the output will also be [0 1]
 */
float KRITAPIGMENT_EXPORT deltaE76(const QVector<float> &a, const QVector<float> &b);

/**
 * @brief XYZToOkLab
 * Ad-hoc implementation of OkLAB. This takes a QVector4D and transforms it to the OkLAB model.
 * @param xyz
 * @return Oklab color.
 */
QVector4D KRITAPIGMENT_EXPORT XYZToOkLab(const QVector4D &xyz);
QVector4D KRITAPIGMENT_EXPORT OkLabToXYZ(const QVector4D &okLab);

/**
 * @brief XYZToICtCp
 * Ad-hoc implementation of ICtCp for use in filters.
 * Unlike the official YCbCr-compatible standard, this one doesn't have a 0.5 offset in the Ct and Cp channels.
 * @param xyz
 * @return
 */
QVector4D KRITAPIGMENT_EXPORT XYZToICtCp(const QVector4D &xyz);
QVector4D KRITAPIGMENT_EXPORT ICtCpToXYZ(const QVector4D &iCtCp);

/**
 * @brief The MappingModel enum
 * This represents the preferred model to do gamut and/or tone mapping in.
 *
 * Here to keep the filter and the transform in sync.
 */
enum MappingModel {
    LAB,
    ICtCp,
    OkLAB
};

/**
 * @brief getGamutCuspDescription
 * Generates a hexagonal gamut cusp description from a RGB or CMYK profile.
 * @param profile cmyk or RGB profile to use.
 * @return gamut cusp description, consisting of hue and the XYZ of the maximum chroma.
 */
QMap<float, QPointF> KRITAPIGMENT_EXPORT getGamutCuspDescription(const KoColorProfile *profile, KoGamutMappingUtils::MappingModel model, const int samplesPerChannel = 5);
QPointF KRITAPIGMENT_EXPORT cuspForHue(const QMap<float, QPointF> &gamutCuspDescription, const float hue);


enum CurveType {
    Reinhard,
    PiecewiseFilmic
};

/**
 * @brief reinhardToneMapping
 * Implements 'extended' reinhard tonemapping.
 * @param linearY -- luminance, linear.
 * @param maxWhite -- maximum white to scale down, also linear
 * @return tone mapped value
 */
float KRITAPIGMENT_EXPORT reinhardToneMapping(const float linearY, const float maxY);

/**
 * @brief The piecewiseFilmicCurve class
 * Implement's Hable's piecewise filmic curve.
 * https://filmicworlds.com/blog/filmic-tonemapping-with-piecewise-power-curves/
 * License: CC-0
 */
struct KRITAPIGMENT_EXPORT PiecewiseFilmicCurve {

    QPointF p0;
    QPointF p1;

    float inverseMaxWhite {1.0};

    struct Segment {
        float lnA{0.0f};
        float b{0.0f};

        float offsetX{0.0f};
        float offsetY{0.0f};
        float scaleX{1.0f};
        float scaleY{1.0f};

        float evalSegment(const float x);
    };

    QVector<Segment> segments;

    PiecewiseFilmicCurve(float maxWhite, QPointF point0, QPointF point1, QPointF overshootPoint);
    static PiecewiseFilmicCurve fromUserParams(float toeStrength = 0.5, float toeLength = 0.5, float shoulderStrength = 2.0, float shoulderLength = 0.5, float shoulderAngle = 1.0);

    float evalCurve(const float x);
};



};

#endif // KOGAMUTMAPPINGUTILS_H
