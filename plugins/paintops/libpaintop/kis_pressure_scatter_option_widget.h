/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_PRESSURE_SCATTER_OPTION_WIDGET_H
#define KIS_PRESSURE_SCATTER_OPTION_WIDGET_H

#include "kis_curve_option_widget.h"


class QCheckBox;

class PAINTOP_EXPORT KisPressureScatterOptionWidget: public KisCurveOptionWidget
{
    Q_OBJECT

public:
    KisPressureScatterOptionWidget();

    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private Q_SLOTS:
    void xAxisEnabled(bool enable);
    void yAxisEnabled(bool enable);

private:
    QCheckBox* m_axisX;
    QCheckBox* m_axisY;
};

#endif // KIS_PRESSURE_RATE_OPTION_WIDGET_H
