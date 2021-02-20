/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_PRESSURE_OPACITY_OPTION
#define KIS_PRESSURE_OPACITY_OPTION

#include "kis_curve_option.h"
#include <kritapaintop_export.h>

class KisPainter;
/**
 * The pressure opacity option defines a curve that is used to
 * calculate the effect of pressure on opacity
 */
class PAINTOP_EXPORT KisPressureOpacityOption : public KisCurveOption
{
public:

    KisPressureOpacityOption();

    /**
     * Set the opacity of the painter based on the pressure
     * and the curve (if checked) and return the old opacity
     * of the painter.
     */
    quint8 apply(KisPainter* painter, const KisPaintInformation& info) const;

    qreal  getOpacityf(const KisPaintInformation& info);

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

};

#endif
