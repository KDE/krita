/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_PRESSURE_SIZE_OPTION_H
#define KIS_PRESSURE_SIZE_OPTION_H

#include "kis_curve_option.h"
#include <brushengine/kis_paint_information.h>
#include <kritapaintop_export.h>

class KisPaintopLodLimitations;


/**
 * The pressure opacity option defines a curve that is used to
 * calculate the effect of pressure on the size of the dab
 */
class PAINTOP_EXPORT KisPressureSizeOption : public KisCurveOption
{
public:
    KisPressureSizeOption();
    double apply(const KisPaintInformation & info) const;

    void lodLimitations(KisPaintopLodLimitations *l) const override;

};

#endif
