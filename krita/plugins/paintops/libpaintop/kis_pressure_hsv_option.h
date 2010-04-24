/* This file is part of the KDE project
 * Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KIS_PRESSURE_HSV_OPTION_H
#define KIS_PRESSURE_HSV_OPTION_H

#include "kis_curve_option.h"
#include <kis_types.h>
#include <krita_export.h>
#include <KoColor.h>

class KoColorTransformation;
class KisColorSource;

/**
 * The pressure opacity option defines a curve that is used to
 * calculate the effect of pressure on one of the hsv of the dab
 */
class PAINTOP_EXPORT KisPressureHSVOption : public KisCurveOption
{
public:
    static KisPressureHSVOption* createHueOption();
    static KisPressureHSVOption* createSaturationOption();
    static KisPressureHSVOption* createValueOption();
public:

    KisPressureHSVOption(const QString& name, const QString& parameterName, double min, double max);

    void apply(KoColorTransformation* hsvTransfo, const KisPaintInformation& info) const;
private:
    struct Private;
    Private* const d;
};

#endif
