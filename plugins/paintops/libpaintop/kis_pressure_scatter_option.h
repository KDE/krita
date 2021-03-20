/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_PRESSURE_SCATTER_OPTION_H
#define KIS_PRESSURE_SCATTER_OPTION_H

#include "kis_curve_option.h"
#include <brushengine/kis_paint_information.h>
#include <kritapaintop_export.h>
#include <kis_types.h>


const QString SCATTER_X = "Scattering/AxisX";
const QString SCATTER_Y = "Scattering/AxisY";
const QString SCATTER_AMOUNT = "Scattering/Amount";

/**
 * Scatters the position of the dab
 */
class PAINTOP_EXPORT KisPressureScatterOption : public KisCurveOption
{
public:
    KisPressureScatterOption();

    QPointF apply(const KisPaintInformation& info, qreal width, qreal height) const;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

    void enableAxisY(bool enable);
    void enableAxisX(bool enable);
    bool isAxisXEnabled();
    bool isAxisYEnabled();
    void setScatterAmount(qreal amount);
    qreal scatterAmount();

private:
    bool m_axisX;
    bool m_axisY;
};

#endif
