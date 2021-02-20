/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _KIS_PERCHANNEL_FILTER_H_
#define _KIS_PERCHANNEL_FILTER_H_

#include <QPair>
#include <QList>

#include <filter/kis_color_transformation_filter.h>
#include <filter/kis_color_transformation_configuration.h>
#include <kis_config_widget.h>
#include <kis_paint_device.h>

#include "virtual_channel_info.h"

#include "kis_multichannel_filter_base.h"

class KisPerChannelFilterConfiguration
        : public KisMultiChannelFilterConfiguration
{
public:
    KisPerChannelFilterConfiguration(int channelCount, KisResourcesInterfaceSP resourcesInterface);
    KisPerChannelFilterConfiguration(const KisPerChannelFilterConfiguration &rhs);
    ~KisPerChannelFilterConfiguration() override;

    KisFilterConfigurationSP clone() const override;

    KisCubicCurve getDefaultCurve() override;
};


/**
 * This class is a filter to adjust channels independently
 */
class KisPerChannelFilter : public KisMultiChannelFilter
{
public:
    KisPerChannelFilter();

    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
    KisFilterConfigurationSP factoryConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;

    KoColorTransformation* createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const override;

    static inline KoID id() {
        return KoID("perchannel", i18n("Color Adjustment"));
    }
};

class KisPerChannelConfigWidget : public KisMultiChannelConfigWidget
{
    Q_OBJECT

public:
    KisPerChannelConfigWidget(QWidget * parent, KisPaintDeviceSP dev, Qt::WindowFlags f = Qt::WindowFlags());
    ~KisPerChannelConfigWidget() override;

    KisPropertiesConfigurationSP configuration() const override;

protected:
    void updateChannelControls() override;

    virtual KisPropertiesConfigurationSP getDefaultConfiguration() override;
};

#endif
