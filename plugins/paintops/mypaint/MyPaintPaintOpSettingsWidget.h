/*
 * SPDX-FileCopyrightText: 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_MYPAINTOP_SETTINGS_WIDGET_H_
#define KIS_MYPAINTOP_SETTINGS_WIDGET_H_

#include <kis_paintop_settings_widget.h>

class MyPaintCurveOptionWidget;

class KisMyPaintOpSettingsWidget : public KisPaintOpSettingsWidget
{
    Q_OBJECT
public:
    enum MyPaintPaintopCategory { BASIC, AIRBRUSH, COLOR, SPEED, DABS, OPACITY, TRACKING, STROKE, SMUDGE, CUSTOM };
public:
    KisMyPaintOpSettingsWidget(QWidget* parent = 0);
    ~KisMyPaintOpSettingsWidget() override;

    KisPropertiesConfigurationSP configuration() const override;

    lager::reader<qreal> effectiveBrushSize() const override;

protected:
    void addPaintOpOption(KisPaintOpOption *option, MyPaintPaintopCategory id);

private:
    MyPaintCurveOptionWidget *m_radiusWidget {nullptr};
};

#endif
