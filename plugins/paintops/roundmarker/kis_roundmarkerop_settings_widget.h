/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_ROUNDMARKEROP_SETTINGS_WIDGET_H_
#define KIS_ROUNDMARKEROP_SETTINGS_WIDGET_H_

#include <kis_paintop_settings_widget.h>

class KisSmudgeOptionWidget;

class KisRoundMarkerOpSettingsWidget : public KisPaintOpSettingsWidget
{
    Q_OBJECT

public:
    KisRoundMarkerOpSettingsWidget(QWidget* parent = 0);
    ~KisRoundMarkerOpSettingsWidget() override;

    KisPropertiesConfigurationSP configuration() const override;

private:
    KisSmudgeOptionWidget *m_smudgeOptionWidget;
};



#endif // KIS_ROUNDMARKEROP_SETTINGS_WIDGET_H_
