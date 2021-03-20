/*
 *  SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_EXPERIMENTPAINTOP_SETTINGS_WIDGET_H_
#define KIS_EXPERIMENTPAINTOP_SETTINGS_WIDGET_H_

#include <kis_paintop_settings_widget.h>


class KisExperimentPaintOpSettingsWidget : public KisPaintOpSettingsWidget
{
    Q_OBJECT
public:
    KisExperimentPaintOpSettingsWidget(QWidget* parent = 0);
    ~KisExperimentPaintOpSettingsWidget() override;

    KisPropertiesConfigurationSP configuration() const override;
};

#endif
