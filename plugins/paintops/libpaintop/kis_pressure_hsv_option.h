/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_PRESSURE_HSV_OPTION_H
#define KIS_PRESSURE_HSV_OPTION_H

#include "kis_curve_option.h"
#include <kis_types.h>
#include <kritapaintop_export.h>
#include <KoColor.h>

class KoColorTransformation;

/**
 * The pressure opacity option defines a curve that is used to
 * calculate the effect of pressure on one of the hsv of the dab
 */
class PAINTOP_EXPORT KisPressureHSVOption : public KisCurveOption
{
public:
    static KisPressureHSVOption* createHueOption();
    static QString hueMinLabel();
    static QString huemaxLabel();

    static KisPressureHSVOption* createSaturationOption();
    static QString saturationMinLabel();
    static QString saturationmaxLabel();

    static KisPressureHSVOption* createValueOption();
    static QString valueMinLabel();
    static QString valuemaxLabel();
public:
    KisPressureHSVOption(const KoID &id);
    ~KisPressureHSVOption() override;

    void apply(KoColorTransformation* hsvTransfo, const KisPaintInformation& info) const;

    // KisCurveOption interface
public:
    int intMinValue() const override;
    int intMaxValue() const override;
    QString valueSuffix() const override;

private:
    struct Private;
    Private* const d;
};

#endif
