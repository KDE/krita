/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_PRESSURE_SHARPNESS_OPTION_H
#define KIS_PRESSURE_SHARPNESS_OPTION_H

#include "kis_curve_option.h"
#include <brushengine/kis_paint_information.h>
#include <kritapaintop_export.h>
#include <kis_types.h>

const QString SHARPNESS_FACTOR = "Sharpness/factor";
const QString SHARPNESS_SOFTNESS  = "Sharpness/softness";

/**
 * This option is responsible to mimic pencil effect from former Pixel Pencil brush engine.auto
 */
class PAINTOP_EXPORT KisPressureSharpnessOption : public KisCurveOption
{
public:
    KisPressureSharpnessOption();

    /**
    * First part of the sharpness is the coordinates: in pen mode they are integers without fractions
    */
    void apply(const KisPaintInformation &info, const QPointF &pt, qint32 &x, qint32 &y, qreal &xFraction, qreal &yFraction) const;

    /**
    * Apply threshold specified by user
    */
    void applyThreshold(KisFixedPaintDeviceSP dab, const KisPaintInformation &info);

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

    /// threshold has 100 levels (like opacity)
    void setThreshold(qint32 threshold) {
        m_softness = qBound<quint32>(0, quint32(threshold), 100);
    }
    qint32 threshold() {
        return qint32(m_softness);
    }

    void setSharpnessFactor(qreal factor) {
        KisCurveOption::setValue(factor);
    }
    qreal sharpnessFactor() {
        return KisCurveOption::value();
    }

private:
    quint32 m_softness {0};
};

#endif
