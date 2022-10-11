/*
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 * SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCURVEOPTION2_H
#define KISCURVEOPTION2_H

#include <vector>
#include <memory>

#include <KisCurveOptionData.h>
#include "sensors2/KisDynamicSensor2.h"

class PAINTOP_EXPORT KisCurveOption2
{
public:
    KisCurveOption2(const KisCurveOptionData &data);

    struct ValueComponents {
        qreal constant {1.0};
        qreal scaling {1.0};
        qreal additive {0.0};
        qreal absoluteOffset {0.0};
        bool hasAbsoluteOffset {false};
        bool hasScaling {false};
        bool hasAdditive {false};
        qreal minSizeLikeValue {0.0};
        qreal maxSizeLikeValue {0.0};

        /**
         * @param normalizedBaseAngle canvas rotation angle normalized to range [0; 1]
         * @param absoluteAxesFlipped true if underlying image coordinate system is flipped (horiz. mirror != vert. mirror)
         */

        qreal rotationLikeValue(qreal normalizedBaseAngle, bool absoluteAxesFlipped, qreal scalingPartCoeff, bool disableScalingPart) const;

        qreal sizeLikeValue() const;
    };

    /**
     * Uses the curves set on the sensors to compute a single
     * double value that can control the parameters of a brush.
     *
     * This value is derives from the values stored in
     * ValuesComponents object.
     */
    ValueComponents computeValueComponents(const KisPaintInformation& info, bool useStrengthValue) const;

    qreal computeSizeLikeValue(const KisPaintInformation &info, bool useStrengthValue = true) const;
    qreal computeRotationLikeValue(const KisPaintInformation& info, qreal baseValue, bool absoluteAxesFlipped, qreal scalingPartCoeff, bool disableScalingPart) const;

    qreal strengthValue() const;
    qreal strengthMinValue() const;
    qreal strengthMaxValue() const;

    bool isChecked() const;

private:
    bool m_isChecked;
    bool m_useCurve;
    int m_curveMode;
    bool m_separateCurveValue;
    qreal m_strengthValue;
    qreal m_strengthMinValue;
    qreal m_strengthMaxValue;
    std::vector<std::unique_ptr<KisDynamicSensor2>> m_sensors;
};

#endif // KISCURVEOPTION2_H
