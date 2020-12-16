/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_PRESSURE_TEXTURE_STRENGTH_OPTION_H
#define __KIS_PRESSURE_TEXTURE_STRENGTH_OPTION_H

#include "kis_curve_option.h"
#include <brushengine/kis_paint_information.h>
#include <kritapaintop_export.h>

/**
 * This curve defines how deep the ink (or a pointer) of a brush
 * penetrates the surface of the canvas, that is how strong we
 * press on the paper
 */
class PAINTOP_EXPORT KisPressureTextureStrengthOption : public KisCurveOption
{
public:
    KisPressureTextureStrengthOption();
    double apply(const KisPaintInformation & info) const;
};

#endif /* __KIS_PRESSURE_TEXTURE_STRENGTH_OPTION_H */
