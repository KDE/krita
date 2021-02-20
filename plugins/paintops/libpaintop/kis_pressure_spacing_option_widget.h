/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_PRESSURE_SPACING_OPTION_WIDGET_H
#define KIS_PRESSURE_SPACING_OPTION_WIDGET_H

#include "kis_curve_option_widget.h"

class QCheckBox;

class PAINTOP_EXPORT KisPressureSpacingOptionWidget : public KisCurveOptionWidget
{
    Q_OBJECT

public:
    KisPressureSpacingOptionWidget();

    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private Q_SLOTS:
    void setIsotropicSpacing(int isotropic);
    void setUseSpacingUpdates(int useSpacingUpdates);

private:
    QCheckBox *m_isotropicSpacing;
    QCheckBox *m_useSpacingUpdates;
};

#endif // KIS_PRESSURE_SPACING_OPTION_WIDGET_H
