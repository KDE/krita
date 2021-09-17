/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Frederic Coiffier <fcoiffie@gmail.com>
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_LEVELS_FILTER_H
#define KIS_LEVELS_FILTER_H

#include <kis_config_widget.h>
#include <filter/kis_color_transformation_filter.h>

#include "KisLevelsFilterConfiguration.h"

class KisLevelsFilter : public KisColorTransformationFilter
{
public:
    KisLevelsFilter();

    KisFilterConfigurationSP factoryConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;

    KoColorTransformation* createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const override;

    static inline KoID id()
    {
        return KoID(KisLevelsFilterConfiguration::defaultName(), i18n("Levels"));
    }
};

#endif
