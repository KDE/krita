/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Frederic Coiffier <fcoiffie@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <cmath>

#include <KoColorSpace.h>
#include <KoColorTransformation.h>
#include <kis_paint_device.h>
#include <filter/kis_filter_category_ids.h>

#include "KisLevelFilter.h"
#include "KisLevelConfigWidget.h"

KisLevelFilter::KisLevelFilter()
        : KisColorTransformationFilter(id(), FiltersCategoryAdjustId, i18n("&Levels..."))
{
    setShortcut(QKeySequence(Qt::CTRL + Qt::Key_L));
    setSupportsPainting(false);
    setColorSpaceIndependence(TO_LAB16);
}

KisLevelFilter::~KisLevelFilter()
{
}

KisConfigWidget * KisLevelFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool) const
{
    return new KisLevelConfigWidget(parent, dev);
}

KoColorTransformation* KisLevelFilter::createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const
{
    if (!config) {
        warnKrita << "No configuration object for level filter\n";
        return 0;
    }

    Q_ASSERT(config);

    int blackvalue = config->getInt("blackvalue");
    int whitevalue = config->getInt("whitevalue", 255);
    double gammavalue = config->getDouble("gammavalue", 1.0);
    int outblackvalue = config->getInt("outblackvalue");
    int outwhitevalue = config->getInt("outwhitevalue", 255);

    quint16 transfer[256];
    for (int i = 0; i < 256; i++) {
        if (i <= blackvalue)
            transfer[i] = outblackvalue;
        else if (i < whitevalue) {
            double a = (double)(i - blackvalue) / (double)(whitevalue - blackvalue);
            a = (double)(outwhitevalue - outblackvalue) * std::pow(a, (1.0 / gammavalue));
            transfer[i] = int(outblackvalue + a);
        } else
            transfer[i] = outwhitevalue;
        // TODO use floats instead of integer in the configuration
        transfer[i] = ((int)transfer[i] * 0xFFFF) / 0xFF ;
    }
    return cs->createBrightnessContrastAdjustment(transfer);
}
