/*
 * SPDX-FileCopyrightText: 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_MYPAINTOP_SETTINGS_WIDGET_H_
#define KIS_MYPAINTOP_SETTINGS_WIDGET_H_

#include <kis_paintop_settings_widget.h>
#include <MyPaintPaintOpOption.h>
#include <MyPaintCurveOptionWidget.h>

class KisMyPaintOpSettingsWidget : public KisPaintOpSettingsWidget
{
    Q_OBJECT

public:
    KisMyPaintOpSettingsWidget(QWidget* parent = 0);
    ~KisMyPaintOpSettingsWidget() override;

    KisPropertiesConfigurationSP configuration() const override;
    void refreshBaseOption();

protected:
    void showEvent(QShowEvent *event) override;

    void addPaintOpOption(KisPaintOpOption *option, KisMyPaintOpOption::PaintopCategory id);

private:
    KisMyPaintOpOption *m_baseOption;
    KisMyPaintCurveOptionWidget *m_radiusWidget;
    KisMyPaintCurveOptionWidget *m_hardnessWidget;
    KisMyPaintCurveOptionWidget *m_opacityWidget;

public Q_SLOTS:
    void updateBaseOptionRadius(qreal);
    void updateBaseOptionHardness(qreal);
    void updateBaseOptionOpacity(qreal);

    void updateRadiusOptionOpacity(qreal);
    void updateHardnessOptionOpacity(qreal);
    void updateOpacityOptionOpacity(qreal);
};

#endif
