/*
 * SPDX-FileCopyrightText: 2014 Manuel Riecke <spell1337@gmail.com>
 *
 * SPDX-License-Identifier: ICS
 */

#pragma once

#include <QColor>
#include "indexcolorpalette.h"

struct PaletteGeneratorConfig
{
    QColor colors[4][4];
    bool   colorsEnabled[4][4];
    int    gradientSteps[3];
    int    inbetweenRampSteps;
    bool   diagonalGradients;

    PaletteGeneratorConfig();
    QByteArray toByteArray();
    void fromByteArray(const QByteArray& str);
    IndexColorPalette generate();
};
