/*
 * Copyright (C) Peter Schatz <voronwe13@gmail.com>, (C) 2020
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
