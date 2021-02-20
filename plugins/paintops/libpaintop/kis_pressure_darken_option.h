/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_PRESSURE_DARKEN_OPTION_H
#define KIS_PRESSURE_DARKEN_OPTION_H

#include "kis_curve_option.h"
#include <kis_types.h>
#include <kritapaintop_export.h>
#include <KoColor.h>

class KisPainter;
class KisColorSource;

/**
 * The pressure opacity option defines a curve that is used to
 * calculate the effect of pressure on the darkness of the dab
 */
class PAINTOP_EXPORT KisPressureDarkenOption : public KisCurveOption
{
public:

    KisPressureDarkenOption();

    KoColor apply(KisPainter * painter, const KisPaintInformation& info) const;
    void apply(KisColorSource* colorSource, const KisPaintInformation& info) const;
};

#endif
