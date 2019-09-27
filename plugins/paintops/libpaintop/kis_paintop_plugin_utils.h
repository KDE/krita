/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __KIS_PAINTOP_PLUGIN_UTILS_H
#define __KIS_PAINTOP_PLUGIN_UTILS_H

#include "kis_paint_information.h"
#include "kis_paintop_utils.h"
#include "kis_paintop_settings.h"
#include "kis_airbrush_option_widget.h"
#include "kis_pressure_spacing_option.h"
#include "kis_pressure_rate_option.h"

namespace KisPaintOpPluginUtils {

/**
 * Similar to KisPaintOpUtils::effectiveSpacing, but some of the required parameters are obtained
 * from the provided configuration options. This function assumes a common configuration where
 * spacing and airbrush settings are configured through a KisPressureSpacingOption and
 * KisAirbrushOption. This type of configuration is used by several different paintops.
 * @param dabWidth - The dab width.
 * @param dabHeight - The dab height.
 * @param isotropicSpacing - If @c true the spacing should be isotropic.
 * @param rotation - The rotation angle in radians.
 * @param axesFlipped - If @c true the axes should be flipped.
 * @param spacingVal - The spacing value.
 * @param autoSpacingActive - If @c true the autospacing will be activated.
 * @param autoSpacingCoeff - The autospacing coefficient.
 * @param lodScale - The level of details scale.
 * @param airbrushOption - The airbrushing option. Can be null for paintops that don't support
 *                         airbrushing.
 * @param spacingOption - The pressure-curve spacing option. Can be null for paintops that don't
 *                        support pressure-based spacing.
 * @param pi - The paint information.
 * @see KisPaintInformation
 */
KisSpacingInformation effectiveSpacing(qreal dabWidth,
                                       qreal dabHeight,
                                       bool isotropicSpacing,
                                       qreal rotation,
                                       bool axesFlipped,
                                       qreal spacingVal,
                                       bool autoSpacingActive,
                                       qreal autoSpacingCoeff,
                                       qreal lodScale,
                                       const KisAirbrushOptionProperties *airbrushOption,
                                       const KisPressureSpacingOption *spacingOption,
                                       const KisPaintInformation &pi)
{
    // Extract required parameters.
    bool distanceSpacingEnabled = true;
    if (airbrushOption && airbrushOption->enabled) {
        distanceSpacingEnabled = !airbrushOption->ignoreSpacing;
    }
    qreal extraScale = 1.0;
    if (spacingOption && spacingOption->isChecked()) {
        extraScale = spacingOption->apply(pi);
    }

    return KisPaintOpUtils::effectiveSpacing(dabWidth, dabHeight, extraScale,
                                             distanceSpacingEnabled, isotropicSpacing, rotation,
                                             axesFlipped, spacingVal, autoSpacingActive,
                                             autoSpacingCoeff, lodScale);
}

/**
 * Similar to KisPaintOpUtils::effectiveTiming, but some of the required parameters are obtained
 * from the provided configuration options. This function assumes a common configuration where
 * airbrush settings are configured through a KisAirbrushOption and KisPressureRateOption. This type
 * of configuration is used by several different paintops.
 * @param airbrushOption - The airbrushing option. Can be null for paintops that don't support
 *                         airbrushing.
 * @param rateOption - The pressure-curve airbrush rate option. Can be null for paintops that don't
 *                     support a pressure-based airbrush rate.
 * @param pi - The paint information.
 * @see KisPaintInformation
 */
KisTimingInformation effectiveTiming(const KisAirbrushOptionProperties *airbrushOption,
                                     const KisPressureRateOption *rateOption,
                                     const KisPaintInformation &pi)
{
    // Extract required parameters.
    bool timingEnabled = false;
    qreal timingInterval = LONG_TIME;
    if (airbrushOption) {
        timingEnabled = airbrushOption->enabled;
        timingInterval = airbrushOption->airbrushInterval;
    }
    qreal rateExtraScale = 1.0;
    if (rateOption && rateOption->isChecked()) {
        rateExtraScale = rateOption->apply(pi);
    }

    return KisPaintOpUtils::effectiveTiming(timingEnabled, timingInterval, rateExtraScale);
}

}

#endif /* __KIS_PAINTOP_PLUGIN_UTILS_H */
