/*
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_PRESSURE_FLOW_OPACITY_OPTION_WIDGET_H
#define KIS_PRESSURE_FLOW_OPACITY_OPTION_WIDGET_H

#include "kis_pressure_flow_opacity_option.h"
#include "kis_curve_option_widget.h"

class KisDoubleSliderSpinBox;
class KisCurveOptionWidget;

class PAINTOP_EXPORT KisFlowOpacityOptionWidget: public KisCurveOptionWidget
{
    Q_OBJECT

public:
    KisFlowOpacityOptionWidget();

    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private Q_SLOTS:
    void slotSliderValueChanged();

private:
    KisDoubleSliderSpinBox* m_opacitySlider;
};

#endif // KIS_PRESSURE_FLOW_OPACITY_OPTION_WIDGET_H
