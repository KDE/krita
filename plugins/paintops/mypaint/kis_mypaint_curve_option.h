#ifndef KIS_MYPAINT_CURVE_OPTION_H
#define KIS_MYPAINT_CURVE_OPTION_H


#include <QObject>
#include <QVector>

#include "kis_paintop_option.h"
#include "kis_global.h"
#include <brushengine/kis_paint_information.h>
#include "kritapaintop_export.h"
#include "kis_mypaintbrush_option.h"
#include "libmypaint/mypaint-brush.h"

class PAINTOP_EXPORT KisMyPaintCurveOption: public QObject
{
    Q_OBJECT
public:
    KisMyPaintCurveOption(const QString& name,
                   KisPaintOpOption::PaintopCategory category,
                   bool checked,
                   qreal value = 1.0,
                   qreal min = 0.0,
                   qreal max = 1.0);

    virtual ~KisMyPaintCurveOption();

    virtual void writeOptionSetting(KisPropertiesConfigurationSP setting);
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
    KisDynamicOptionSP sensor(MyPaintBrushOptionType sensorType, bool active) const;
    void replaceSensor(KisDynamicOptionSP sensor);
    QList<KisDynamicOptionSP> sensors();
    QList<KisDynamicOptionSP> activeSensors() const;
    MyPaintBrushSetting currentSetting();
    QList<MyPaintBrushInput> inputList();

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
    void setCurve(MyPaintBrushOptionType sensorType, bool useSameCurve, const KisCubicCurve &curve);
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

        qreal rotationLikeValue(qreal normalizedBaseAngle, bool absoluteAxesFlipped) const {
            const qreal offset =
                !hasAbsoluteOffset ? normalizedBaseAngle :
                absoluteAxesFlipped ? 1.0 - absoluteOffset :
                absoluteOffset;

            const qreal realScalingPart = hasScaling ? KisMyPaintBrushOption::scalingToAdditive(scaling) : 0.0;
            const qreal realAdditivePart = hasAdditive ? additive : 0;

            qreal value = wrapInRange(2 * offset + constant * (realScalingPart + realAdditivePart), -1.0, 1.0);
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
            const qreal realAdditivePart = hasAdditive ? KisMyPaintBrushOption::additiveToScaling(additive) : 1.0;

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

    /**
     * Curve that is being used instead of sensors' internal ones
     * in case "Use the same curve" is checked.
     */
    KisCubicCurve m_commonCurve;

    int m_curveMode;  

    QMap<MyPaintBrushOptionType, KisDynamicOptionSP> m_sensorMap;

private:

    qreal m_value;
    qreal m_minValue;
    qreal m_maxValue;
    bool firstRead = true;

Q_SIGNALS:
    void checkUseCurve();
};

#endif // KIS_MYPAINT_CURVE_OPTION_H
