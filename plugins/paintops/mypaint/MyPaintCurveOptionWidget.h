/*
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2009 Sven Langkamp <sven.langkamp@gmail.com>
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 * SPDX-FileCopyrightText: 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_MYPAINT_CURVE_OPTION_WIDGET_H
#define KIS_MYPAINT_CURVE_OPTION_WIDGET_H

#include <QObject>

#include <MyPaintCurveOption.h>
#include <MyPaintPaintOpOption.h>
#include <kis_curve_option_widget.h>
#include <kis_paintop_option.h>
#include <kritapaintop_export.h>

#include "ui_wdgcurveoption.h"
#include "ui_wdgmypaintcurveoption.h"

class Ui_WdgMyPaintCurveOption;
class QComboBox;

class KisMyPaintCurveOptionWidget : public KisCurveOptionWidget
{
    Q_OBJECT
public:
    KisMyPaintCurveOptionWidget(KisMyPaintCurveOption* curveOption, const QString &minLabel, const QString &maxLabel, bool hideSlider = false, KisMyPaintOpOption *option = nullptr);
    ~KisMyPaintCurveOptionWidget() override;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

    KisDoubleSliderSpinBox* slider();

protected Q_SLOTS:

    void slotUnCheckUseCurve();

    void updateSensorCurveLabels(KisDynamicSensorSP sensor) const override;
    void updateRangeSpinBoxes(KisDynamicSensorSP sensor) const;

public Q_SLOTS:
    void refresh();

protected:

    void checkRanges() const;
    float getBaseValue(KisPropertiesConfigurationSP setting);
    void setBaseValue(KisPropertiesConfigurationSP setting, float val) const;

};

#endif // KIS_MYPAINT_CURVE_OPTION_WIDGET_H
