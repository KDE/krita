/*
 *  SPDX-FileCopyrightText: 2008, 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_SPRAYPAINTOP_SETTINGS_WIDGET_H_
#define KIS_SPRAYPAINTOP_SETTINGS_WIDGET_H_

#include <kis_paintop_settings_widget.h>

class KisSprayOpOption;

class KisSprayPaintOpSettingsWidget : public KisPaintOpSettingsWidget
{
    Q_OBJECT

public:
    KisSprayPaintOpSettingsWidget(QWidget* parent = 0);
    ~KisSprayPaintOpSettingsWidget() override;

    KisPropertiesConfigurationSP configuration() const override;
};

#endif
