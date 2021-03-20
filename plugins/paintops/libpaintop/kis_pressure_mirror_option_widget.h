/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_PRESSURE_MIRROR_OPTION_WIDGET_H
#define KIS_PRESSURE_MIRROR_OPTION_WIDGET_H

#include "kis_curve_option_widget.h"

class QCheckBox;

class PAINTOP_EXPORT KisPressureMirrorOptionWidget : public KisCurveOptionWidget
{
    Q_OBJECT

public:
    KisPressureMirrorOptionWidget();

    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private Q_SLOTS:
    void horizontalMirrorChanged(bool mirror);
    void verticalMirrorChanged(bool mirror);

private:
    QCheckBox* m_horizontalMirror;
    QCheckBox* m_verticalMirror;
};

#endif // KIS_PRESSURE_RATE_OPTION_WIDGET_H
