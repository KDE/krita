/*
 * Copyright (C) Peter Schatz <voronwe13@gmail.com>, (C) 2020
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_PRESSURE_LIGHTNESS_STRENGTH_OPTION_WIDGET_H
#define KIS_PRESSURE_LIGHTNESS_STRENGTH_OPTION_WIDGET_H

#include "kis_curve_option_widget.h"

class QLabel;

class PAINTOP_EXPORT KisPressureLightnessStrengthOptionWidget : public KisCurveOptionWidget
{
    Q_OBJECT

public:
    KisPressureLightnessStrengthOptionWidget();

    void setEnabled(bool enabled) override;

private:
    QLabel* m_enabledLabel;
};

#endif // KIS_PRESSURE_LIGHTNESS_STRENGTH_OPTION_WIDGET_H
