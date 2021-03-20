/*
 *  SPDX-FileCopyrightText: 2014 Mohit Goyal <mohit.bits2011@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_SMUDGE_RADIUS_OPTION_H
#define KIS_SMUDGE_RADIUS_OPTION_H
#include "kis_rate_option.h"
#include <brushengine/kis_paint_information.h>
#include <kis_types.h>

class KisPropertiesConfiguration;
class KisPainter;

class KisSmudgeRadiusOption: public KisRateOption
{
public:
    KisSmudgeRadiusOption();

    QRect sampleRect(const KisPaintInformation &info, qreal diameter, const QPoint &pos) const;

    /**
     * Set the opacity of the painter based on the rate
     * and the curve (if checked)
     */
    void apply(KoColor *resultColor,
               const KisPaintInformation& info,
               qreal diameter,
               qreal posx,
               qreal posy,
               KisPaintDeviceSP dev) const;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;



};
#endif // KIS_SMUDGE_RADIUS_OPTION_H
