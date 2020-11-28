/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2011 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_PRESSURE_SPACING_OPTION_H
#define KIS_PRESSURE_SPACING_OPTION_H

#include "kis_curve_option.h"
#include <brushengine/kis_paint_information.h>
#include <kritapaintop_export.h>

/**
 * The pressure spacing option defines a curve that is used to
 * calculate the effect of pressure on the spacing of the dab
 */
class PAINTOP_EXPORT KisPressureSpacingOption : public KisCurveOption
{
public:
    KisPressureSpacingOption();
    double apply(const KisPaintInformation & info) const;

    void setIsotropicSpacing(bool isotropic);
    bool isotropicSpacing() const;

    /**
     * @param useUpdates True if and only if the spacing option should allow spacing updates between
     *                   painted dabs.
     */
    void setUsingSpacingUpdates(bool useUpdates);
    /**
     * @return True if and only if the spacing option allows spacing updates between painted dabs.
     */
    bool usingSpacingUpdates() const;

    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;
    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;

private:
    bool m_isotropicSpacing;
    bool m_useSpacingUpdates;
};

#endif
