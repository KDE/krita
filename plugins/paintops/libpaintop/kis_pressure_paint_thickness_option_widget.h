/*
 * SPDX-FileCopyrightText: 2021 Peter Schatz <voronwe13@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_PRESSURE_PAINT_THICKNESS_OPTION_WIDGET_H
#define KIS_PRESSURE_PAINT_THICKNESS_OPTION_WIDGET_H

#include "kis_curve_option_widget.h"

class QLabel;
class QComboBox;

class PAINTOP_EXPORT KisPressurePaintThicknessOptionWidget : public KisCurveOptionWidget
{
    Q_OBJECT

public:
    KisPressurePaintThicknessOptionWidget();

    void setEnabled(bool enabled) override;

    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private Q_SLOTS:
    void slotCurrentIndexChanged(int index);

private:
    QLabel* m_enabledLabel;
    QComboBox* m_cbThicknessMode;
};

#endif // KIS_PRESSURE_PAINT_THICKNESS_OPTION_WIDGET_H
