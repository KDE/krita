/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
class PAINTOP_EXPORT KisCurveOption: public QObject
{
    Q_OBJECT
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

    void updateRange(qreal minValue, qreal maxValue);

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

    /**
     * Returns the curve that is being used instead of sensor ones
     * in case "Use the same curve" is checked.
     */
    KisCubicCurve getCommonCurve() const;

    void setSeparateCurveValue(bool separateCurveValue);

    void setChecked(bool checked);
    void setCurveUsed(bool useCurve);
    void setCurve(DynamicSensorType sensorType, bool useSameCurve, const KisCubicCurve &curve);
    void setValue(qreal value);
    void setCurveMode(int mode);

    /**
     * Sets the bool indicating whether "Share curve across all settings" is checked.
     */
    void setUseSameCurve(bool useSameCurve);

    /**
     * Sets the curve that is being used instead of sensor ones
     * in case "Share curve across all settings" is checked.
     */
    void setCommonCurve(KisCubicCurve curve);

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
    ValueComponents computeValueComponents(const KisPaintInformation& info) const;

    qreal computeSizeLikeValue(const KisPaintInformation &info) const;
    qreal computeRotationLikeValue(const KisPaintInformation& info, qreal baseValue, bool absoluteAxesFlipped, qreal scalingPartCoeff, bool disableScalingPart) const;

    /**
     * @brief defaultCurve returns a curve that is set when the KisCurveOption is not initialized yet
     * the purpose of distinguishing between this one and emptyCurve() is to allow easier finding out that something is wrong
     * in the code setting common curves
     * @return a non-standard curve with two hills
     */
    KisCubicCurve defaultCurve();
    /**
     * @brief emptyCurve returns the simplest usable curve
     * @return curve from (0, 0) to (1, 1)
     */
    KisCubicCurve emptyCurve();

    virtual QList<KoID> sensorsIds();
    virtual DynamicSensorType id2Type(const KoID &id);
    virtual KisDynamicSensorSP id2Sensor(const KoID& id, const QString &parentOptionName);
    virtual KisDynamicSensorSP type2Sensor(DynamicSensorType sensorType, const QString &parentOptionName);
    virtual QList<DynamicSensorType> sensorsTypes();

protected:

    void setValueRange(qreal min, qreal max);    
    /**
     * Read the option using the prefix in argument
     */
    virtual void readNamedOptionSetting(const QString& prefix, const KisPropertiesConfigurationSP setting);

    QString m_name;
    KisPaintOpOption::PaintopCategory m_category;

    bool m_checkable;
    bool m_checked;
    bool m_useCurve;
    bool m_useSameCurve;
    bool m_separateCurveValue;

    /**
     * Curve that is being used instead of sensors' internal ones
     * in case "Use the same curve" is checked.
     */
    KisCubicCurve m_commonCurve;

    int m_curveMode;

    QMap<DynamicSensorType, KisDynamicSensorSP> m_sensorMap;

    qreal m_value;
    qreal m_minValue;
    qreal m_maxValue;

Q_SIGNALS:
    void unCheckUseCurve();
};

#endif
