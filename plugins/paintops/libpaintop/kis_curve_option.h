/* This file is part of the KDE project
 * Copyright (C) 2008 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2011 Silvio Heinrich <plassy@web.de>
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

#ifndef KIS_CURVE_OPTION_H
#define KIS_CURVE_OPTION_H

#include <QObject>
#include <QVector>

#include "kis_paintop_option.h"
#include "kis_global.h"
#include <brushengine/kis_paint_information.h>
#include "kritapaintop_export.h"
#include "kis_dynamic_sensor.h"

class KisDynamicSensor;

/**
 * KisCurveOption is the base class for paintop options that are
 * defined through one or more curves.
 *
 * Note: it is NOT a KisPaintOpOption, even though the API is pretty similar!
 *
 * KisCurveOption classes have a generic GUI widget, KisCurveOptionWidget. So,
 * in contrast to KisPaintOpOption classes, KisCurveOption instances can and
 * will be created in the constructor of KisPaintOp paintops. This class can
 * manage to read and write its settings directly.
 *
 */
class PAINTOP_EXPORT KisCurveOption
{
public:
    KisCurveOption(const QString& name,
                   KisPaintOpOption::PaintopCategory category,
                   bool checked,
                   qreal value = 1.0,
                   qreal min = 0.0,
                   qreal max = 1.0);

    virtual ~KisCurveOption();

    virtual void writeOptionSetting(KisPropertiesConfigurationSP setting) const;
    virtual void readOptionSetting(KisPropertiesConfigurationSP setting);
    virtual void lodLimitations(KisPaintopLodLimitations *l) const;

    //Please override for other values than 0-100 and %
    virtual int intMinValue()const;
    virtual int intMaxValue()const;
    virtual QString valueSuffix()const;

    const QString& name() const;
    KisPaintOpOption::PaintopCategory category() const;
    qreal minValue() const;
    qreal maxValue() const;
    qreal value() const;

    void resetAllSensors();
    KisDynamicSensorSP sensor(DynamicSensorType sensorType, bool active) const;
    void replaceSensor(KisDynamicSensorSP sensor);
    QList<KisDynamicSensorSP> sensors();
    QList<KisDynamicSensorSP> activeSensors() const;

    bool isCheckable();
    bool isChecked() const;
    bool isCurveUsed() const;
    bool isSameCurveUsed() const;
    bool isRandom() const;

    int getCurveMode() const;

    void setSeparateCurveValue(bool separateCurveValue);

    void setChecked(bool checked);
    void setCurveUsed(bool useCurve);
    void setCurve(DynamicSensorType sensorType, bool useSameCurve, const KisCubicCurve &curve);
    void setValue(qreal value);
    void setCurveMode(int mode);

    struct ValueComponents {

        ValueComponents()
            : constant(1.0),
              scaling(1.0),
              additive(0.0),
              absoluteOffset(0.0),
              hasAbsoluteOffset(false),
              hasScaling(false),
              hasAdditive(false)
        {
        }

        qreal constant;
        qreal scaling;
        qreal additive;
        qreal absoluteOffset;
        bool hasAbsoluteOffset;
        bool hasScaling;
        bool hasAdditive;
        qreal minSizeLikeValue;
        qreal maxSizeLikeValue;

        /**
         * @param normalizedBaseAngle canvas rotation angle normalized to range [0; 1]
         * @param absoluteAxesFlipped true if underlying image coordinate system is flipped (horiz. mirror != vert. mirror)
         */

        qreal rotationLikeValue(qreal normalizedBaseAngle, bool absoluteAxesFlipped) const {
            const qreal offset =
                !hasAbsoluteOffset ? normalizedBaseAngle :
                absoluteAxesFlipped ? 1.0 - absoluteOffset :
                absoluteOffset;

            const qreal realScalingPart = hasScaling ? KisDynamicSensor::scalingToAdditive(scaling) : 0.0;
            const qreal realAdditivePart = hasAdditive ? additive : 0;

            qreal value = wrapInRange(2 * offset + constant * realScalingPart + realAdditivePart, -1.0, 1.0);
            if (qIsNaN(value)) {
                qWarning() << "rotationLikeValue returns NaN!" << normalizedBaseAngle << absoluteAxesFlipped;
                value = 0;
            }
            return value;
        }

        qreal sizeLikeValue() const {
            const qreal offset =
                hasAbsoluteOffset ? absoluteOffset : 1.0;

            const qreal realScalingPart = hasScaling ? scaling : 1.0;
            const qreal realAdditivePart = hasAdditive ? KisDynamicSensor::additiveToScaling(additive) : 1.0;

            return qBound(minSizeLikeValue,
                          constant * offset * realScalingPart * realAdditivePart,
                          maxSizeLikeValue);
        }

    private:
        static inline qreal wrapInRange(qreal x, qreal min, qreal max) {
            const qreal range = max - min;

            x -= min;

            if (x < 0.0) {
                x = range + fmod(x, range);
            }

            if (x > range) {
                x = fmod(x, range);
            }

            return x + min;
        }
    };

    /**
     * Uses the curves set on the sensors to compute a single
     * double value that can control the parameters of a brush.
     *
     * This value is derives from the values stored in
     * ValuesComponents object.
     */
    ValueComponents computeValueComponents(const KisPaintInformation& info) const;

    qreal computeSizeLikeValue(const KisPaintInformation &info) const;
    qreal computeRotationLikeValue(const KisPaintInformation& info, qreal baseValue, bool absoluteAxesFlipped) const;

protected:

    void setValueRange(qreal min, qreal max);

    /**
     * Read the option using the prefix in argument
     */
    void readNamedOptionSetting(const QString& prefix, const KisPropertiesConfigurationSP setting);

    QString m_name;
    KisPaintOpOption::PaintopCategory m_category;

    bool m_checkable;
    bool m_checked;
    bool m_useCurve;
    bool m_useSameCurve;
    bool m_separateCurveValue;

    int m_curveMode;

    QMap<DynamicSensorType, KisDynamicSensorSP> m_sensorMap;
    QMap<DynamicSensorType, KisCubicCurve> m_curveCache;

private:

    qreal m_value;
    qreal m_minValue;
    qreal m_maxValue;
};

#endif
