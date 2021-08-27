/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Frederic Coiffier <fcoiffie@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_LEVEL_FILTER_H_
#define _KIS_LEVEL_FILTER_H_

#include "filter/kis_color_transformation_filter.h"
#include "kis_config_widget.h"

/**
 * This class affect Intensity Y of the image
 */
class KisLevelFilter : public KisColorTransformationFilter
{
public:
    KisLevelFilter();
    ~KisLevelFilter() override;

//     virtual KisFilterConfigurationSP factoryConfiguration() const;
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;

    KoColorTransformation* createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const override;

    static inline KoID id() {
        return KoID("levels", i18n("Levels"));
    }
};

#endif
