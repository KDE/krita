/*
 * SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef KOGAMUTMAPPINGUTILS_H
#define KOGAMUTMAPPINGUTILS_H

#include <QVector>
#include "kritapigment_export.h"

namespace KRITAPIGMENT_EXPORT KoGamutMappingUtils {

/**
 * @brief deltaE76
 * calculates the difference between two colors, represented as normalized channels.
 * @return The difference. Given that the input is scaled to [0 1], the output will also be [0 1]
 */
float deltaE76(const QVector<float> &a, const QVector<float> &b);

/**
 * @brief deltaE2000
 * Like deltaE76, but instead this uses a more sophisticated algorithm, to compensate for various
 * issues in the LAB color mode.
 * @return The difference. Given that the input is scaled to [0 1], the output will also be [0 1]
 */
float deltaE2000(const QVector<float> &a, const QVector<float> &b);

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
 * @brief reinhardToneMapping
 * Implements 'extended' reinhard tonemapping.
 * @param linearY -- luminance, linear.
 * @param maxWhite -- maximum white to scale down, also linear
 * @return tone mapped value
 */
float reinhardToneMapping(const float linearY, const float maxY);

};

#endif // KOGAMUTMAPPINGUTILS_H
