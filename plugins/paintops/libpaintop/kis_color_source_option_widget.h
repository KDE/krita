/*
 * SPDX-FileCopyrightText: 2011 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_COLOR_SOURCE_OPTION_WIDGET_H
#define KIS_COLOR_SOURCE_OPTION_WIDGET_H

#include "kis_paintop_option.h"
#include <kritapaintop_export.h>

class KisPaintopLodLimitations;

/**
 * The brush option allows the user to select a particular brush
 * footprint for suitable paintops
 */
class PAINTOP_EXPORT KisColorSourceOptionWidget : public KisPaintOpOption
{
    Q_OBJECT
public:
    KisColorSourceOptionWidget();
    ~KisColorSourceOptionWidget() override;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

    void lodLimitations(KisPaintopLodLimitations *l) const override;

private Q_SLOTS:
    void sourceChanged();
private:
    struct Private;
    Private* const d;
};

#endif
