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
#include "kis_pressure_spacing_option.h"
#include "kis_pressure_rate_option.h"

namespace KisPaintOpPluginUtils {

/**
 * Similar to KisPaintOpUtils::effectiveSpacing, but some of the required parameters are obtained
 * from the provided configuration options. This function assumes a common configuration where
 * curve-based spacing and rate options provide pressure sensitivity, and airbrush settings are
 * configured through a KisPropertiesConfiguration. This type of configuration is used by several
 * different paintops.
 * @param propsConfig - The properties configuration for the paintop. Can be null to use a default
 *                      configuration.
 * @param spacingOption - The pressure-curve spacing option. Can be null for paintops that don't
 *                        support pressure-based spacing.
 * @param rateOption - The pressure-curve airbrush rate option. Can be null for paintops that don't
 *                     support a pressure-based airbrush rate.
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
                                       const KisPropertiesConfigurationSP propsConfig,
                                       const KisPressureSpacingOption *spacingOption,
                                       const KisPressureRateOption *rateOption,
                                       const KisPaintInformation &pi)
{
    // Extract required parameters.
    // Note: timedSpacingInterval might end up being infinity. That shouldn't cause any problems.
    bool distanceSpacingEnabled = true;
    bool timedSpacingEnabled = false;
    qreal timedSpacingInterval = 0.0;
    if (propsConfig) {
        timedSpacingEnabled = propsConfig->getBool(AIRBRUSH_ENABLED, false);
        distanceSpacingEnabled
                = !(timedSpacingEnabled && propsConfig->getBool(AIRBRUSH_IGNORE_SPACING, false));
        qreal timedSpacingRate = propsConfig->getDouble(AIRBRUSH_RATE, 1.0);
        if (timedSpacingRate == 0.0) {
            timedSpacingRate = 1.0;
        }
        timedSpacingInterval = 1000.0 / timedSpacingRate;
    }
    qreal extraScale = 1.0;
    if (spacingOption && spacingOption->isChecked()) {
        extraScale = spacingOption->apply(pi);
    }
    qreal rateExtraScale = 1.0;
    if (rateOption && rateOption->isChecked()) {
        rateExtraScale = rateOption->apply(pi);
    }

    return KisPaintOpUtils::effectiveSpacing(dabWidth, dabHeight, extraScale, rateExtraScale,
                                             distanceSpacingEnabled, isotropicSpacing, rotation,
                                             axesFlipped, spacingVal, autoSpacingActive,
                                             autoSpacingCoeff, timedSpacingEnabled,
                                             timedSpacingInterval, lodScale);
}

}

#endif /* __KIS_PAINTOP_PLUGIN_UTILS_H */
