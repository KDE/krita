/*
 *  SPDX-FileCopyrightText: 2008, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_CURVE_PAINTOP_SETTINGS_WIDGET_H_
#define KIS_CURVE_PAINTOP_SETTINGS_WIDGET_H_

#include <kis_paintop_settings_widget.h>

class KisCurveOpOption;
class KisPropertiesConfiguration;

class KisCurvePaintOpSettingsWidget : public KisPaintOpSettingsWidget
{
    Q_OBJECT

public:
    KisCurvePaintOpSettingsWidget(QWidget* parent = 0);
    ~KisCurvePaintOpSettingsWidget() override;

    KisPropertiesConfigurationSP configuration() const override;
};


#endif
