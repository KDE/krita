/*
 *  SPDX-FileCopyrightText: 2025 Agata Cacko
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef GUTTER_WIDTHS_CONFIG_H_
#define GUTTER_WIDTHS_CONFIG_H_

#include <QScopedPointer>
#include <QPoint>
#include <QRectF>
#include <KoUnit.h>


class GutterWidthsConfig
{
public:
    GutterWidthsConfig(KoUnit _baseUnit, qreal resolution, qreal _horizontal, qreal _vertical, qreal _diagonal, qreal _angleDegrees);
    GutterWidthsConfig(KoUnit _baseUnit, qreal resolution, qreal _all, qreal _angleDegrees);

public:

    qreal widthForAngleInPixels(qreal lineAngleDegrees);

public:


    const KoUnit baseUnit;

    const qreal horizontal;
    const qreal vertical;
    const qreal diagonal;
    const qreal angleDegrees;
    const qreal resolution;


};




#endif // GUTTER_WIDTHS_CONFIG_H_
